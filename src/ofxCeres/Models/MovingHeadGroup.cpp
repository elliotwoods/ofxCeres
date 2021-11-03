#include "pch_ofxCeres.h"
#include "MovingHeadGroup.h"
#include "ofxCeres/VectorMath/VectorMath.h"

struct MovingHeadGroupError {
	MovingHeadGroupError(const glm::tvec2<double>& panTiltValuesSignal)
		: panTiltSignal(panTiltValuesSignal) {}

	template <typename T>
	bool operator()(const T* const translationParameters
		, const T* const rotationParameters
		, const T* const panDistortionParameters
		, const T* const tiltDistortionParameters
		, const T* const targetPositionParameters
		, T* residuals) const {

		// The aim of this residual function is to compare the ray in 3D space with the actual point in 3D space
		
		//--
		// Extract 3D parameters
		glm::tvec3<T> translation(translationParameters[0], translationParameters[1], translationParameters[2]);
		glm::tvec3<T> rotationVector(rotationParameters[0], rotationParameters[1], rotationParameters[2]);
		auto rotation = ofxCeres::VectorMath::eulerToQuat(rotationVector);

		glm::tvec3<T> targetPosition(targetPositionParameters[0], targetPositionParameters[1], targetPositionParameters[2]);
		//
		//--


		//--
		// We don't need to ignore points if they coincide with light fixture since these points will give a residual of 0 anyway
		//
		//--


		//--
		// Get ideal angles for the capture signal values
		glm::tvec2<T> idealPanTiltFromCapture{
			ofxCeres::VectorMath::powerSeries2((T)this->panTiltSignal.x, panDistortionParameters)
			, ofxCeres::VectorMath::powerSeries2((T)this->panTiltSignal.y, tiltDistortionParameters)
		};
		//
		//--
		

		//--
		// Calculate the ray in world space
		auto rayTransmissionInObjectSpace = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt<T>(idealPanTiltFromCapture);
		auto rayTransmissionInWorldspace = rotation * rayTransmissionInObjectSpace;
		//
		//--

		//--
		// Calculate the delta (choose one method from below and set the appropriate residual count in Create below)
		//
		
		// 1. Calculate the distsnce between moving head and target point (given current translation
		//const auto& position = translation; // This is true for ofxCeres::VectorMath::createTransform
		//auto distance = ofxCeres::VectorMath::distanceRayToPoint(position, rayTransmissionInWorldspace, targetPosition);
		//residuals[0] = distance;
		
		// 2. Unproject the ray at the distance
		//auto distanceToTarget = ofxCeres::VectorMath::distance(translation, targetPosition);
		//auto rayNearTarget = rayTransmissionInWorldspace * distanceToTarget / ofxCeres::VectorMath::length(rayTransmissionInWorldspace) + translation;
		//auto delta = rayNearTarget - targetPosition;
		//residuals[0] = delta[0];
		//residuals[1] = delta[1];
		//residuals[2] = delta[2];
		 
		// 3. Difference between the ray transmissions
		//auto delta = glm::normalize(targetPosition - translation) - glm::normalize(rayTransmissionInWorldspace);
		//residuals[0] = delta[0];
		//residuals[1] = delta[1];
		//residuals[2] = delta[2];

		// 4. Take difference between ray angles
		const auto& movingHeadPosition = translation;
		const auto directionTowardsPoint = ofxCeres::VectorMath::normalize(targetPosition - movingHeadPosition);
		auto angleBetween = acos(ofxCeres::VectorMath::dot(directionTowardsPoint, rayTransmissionInWorldspace));

		residuals[0] = angleBetween * angleBetween;
		//
		//--


		return true;
	}

	static ceres::CostFunction* Create(const glm::tvec2<double>& panTiltSignal) {
		return new ceres::AutoDiffCostFunction<MovingHeadGroupError, 1, 3, 3, 3, 3, 3>(
			new MovingHeadGroupError(panTiltSignal)
			);
	}

	const glm::tvec2<double> panTiltSignal;
};

struct PointInPlaneError {
	PointInPlaneError(const glm::tvec4<double>& plane)
		: plane(plane) {}

	template <typename T>
	bool operator()(const T* const pointPosition
		, T* residuals) const {
		glm::tvec4<T> vec4(pointPosition[0], pointPosition[1], pointPosition[2], 1.0);
		residuals[0] = glm::dot(vec4, (glm::tvec4<T>) this->plane);
		return true;
	}

	static ceres::CostFunction* Create(const glm::tvec4<double>& plane) {
		return new ceres::AutoDiffCostFunction<PointInPlaneError, 1, 3>(
			new PointInPlaneError(plane)
			);
	}

	const glm::tvec4<double> plane;
};


namespace ofxCeres {
	namespace Models {
		//----------
		MovingHeadGroup::Result
			MovingHeadGroup::solve(const std::vector<Image>& images
				, const Solution& initialSolution
				, const std::vector<shared_ptr<Constraint>>& constraints
				, const Options& options
				, const SolverSettings& solverSettings) {
			try {
				// Check some errors
				if (initialSolution.movingHeads.size() != images.size()) {
					throw(ofxCeres::Exception("initialSolution.movingHeads.size() != images.size()"));
				}
				if (images.empty()) {
					throw(ofxCeres::Exception("images.empty()"));
				}


				// Load the markerPositions into parameters
				vector<array<double, 3>> markerPositionParameters;
				for (const auto& markerPosition : initialSolution.markerPositions) {
					markerPositionParameters.push_back({
						markerPosition[0]
						, markerPosition[1]
						, markerPosition[2]
						});
				}

				// Load the moving heads into parameters
				vector<array<double, 3>> translationParameters;
				vector<array<double, 3>> rotationVectorParameters;
				vector<array<double, 3>> panDistortionParameters;
				vector<array<double, 3>> tiltDistortionParameters;
				for (const auto& movingHead : initialSolution.movingHeads) {
					translationParameters.push_back({
						movingHead.basicSolution.translation[0]
						, movingHead.basicSolution.translation[1]
						, movingHead.basicSolution.translation[2]
						});
					rotationVectorParameters.push_back({
						movingHead.basicSolution.rotationVector[0]
						, movingHead.basicSolution.rotationVector[1]
						, movingHead.basicSolution.rotationVector[2]
						});
					panDistortionParameters.push_back({
						movingHead.panDistortion[0]
						, movingHead.panDistortion[1]
						, movingHead.panDistortion[2]
						});
					tiltDistortionParameters.push_back({
						movingHead.tiltDistortion[0]
						, movingHead.tiltDistortion[1]
						, movingHead.tiltDistortion[2]
						});
				}

				// Build up the problem, cost functions
				ceres::Problem problem;
				set<int> activeMarkerIndices;
				for (size_t movingHeadIndex = 0; movingHeadIndex < images.size(); movingHeadIndex++) {
					const auto& image = images[movingHeadIndex];
					auto size = image.panTiltSignal.size();
					for (size_t i = 0; i < size; i++) {
						auto costFunction = MovingHeadGroupError::Create(image.panTiltSignal[i]);
						problem.AddResidualBlock(costFunction
							, NULL
							, translationParameters[movingHeadIndex].data()
							, rotationVectorParameters[movingHeadIndex].data()
							, panDistortionParameters[movingHeadIndex].data()
							, tiltDistortionParameters[movingHeadIndex].data()
							, markerPositionParameters[image.markerIndex[i]].data()
						);
						activeMarkerIndices.insert(image.markerIndex[i]);
					}
				}

				// Apply constraints
				for (auto constraint : constraints) {
					// Check that the marker is part of the problem before applying the constriant
					auto markerIsActive = activeMarkerIndices.find(constraint->markerIndex) != activeMarkerIndices.end();
					if (!markerIsActive) {
						continue;
					}
					auto markerPositionParameter = markerPositionParameters[constraint->markerIndex].data();

					{
						auto fixedMarkerConstraint = dynamic_pointer_cast<FixedMarkerConstraint>(constraint);
						if (fixedMarkerConstraint) {
							problem.SetParameterBlockConstant(markerPositionParameter);
							continue;
						}
					}

					{
						auto markerInPlaneConstraint = dynamic_pointer_cast<MarkerInPlaneConstraint>(constraint);
						if (markerInPlaneConstraint) {
							auto costFunction = PointInPlaneError::Create((glm::tvec4<double>) markerInPlaneConstraint->abcd);
							problem.AddResidualBlock(
								costFunction
								, NULL
								, markerPositionParameter);
							continue;
						}
					}
				}

				// Apply options
				{
					if (options.noDistortion) {
						for (auto& parameters : panDistortionParameters) {
							for (size_t i = 0; i < parameters.size(); i++) {
								if (i == 1) {
									parameters[i] == 1.0;
								}
								else {
									parameters[i] = 0.0;
								}
							}
							problem.SetParameterBlockConstant(parameters.data());
						}
						for (auto& parameters : tiltDistortionParameters) {
							for (size_t i = 0; i < parameters.size(); i++) {
								if (i == 1) {
									parameters[i] == 1.0;
								}
								else {
									parameters[i] = 0.0;
								}
							}
							problem.SetParameterBlockConstant(parameters.data());
						}

					}
				}

				ceres::Solver::Summary summary;
				ceres::Solve(solverSettings.options, &problem, &summary);

				if (solverSettings.printReport) {
					std::cout << summary.FullReport() << "\n";
				}

				// construct result
				{
					// This is the count of marker projections
					auto size = problem.NumResidualBlocks();

					Result result(summary, sqrt(summary.final_cost / (double)size));
					
					// Pull the markerPositions into solution
					{
						for (auto& markerPositionParameter : markerPositionParameters) {
							result.solution.markerPositions.push_back(glm::vec3(
								markerPositionParameter[0]
								, markerPositionParameter[1]
								, markerPositionParameter[2]
							));
						}
					}

					// Pull the moving head parameters into solution
					{
						for (size_t movingHeadIndex = 0; movingHeadIndex < images.size(); movingHeadIndex++) {
							auto translation = glm::vec3({
								translationParameters[movingHeadIndex][0]
								, translationParameters[movingHeadIndex][1]
								, translationParameters[movingHeadIndex][2]
								});
							auto rotationVector = glm::vec3({
								rotationVectorParameters[movingHeadIndex][0]
								, rotationVectorParameters[movingHeadIndex][1]
								, rotationVectorParameters[movingHeadIndex][2]
								});
							auto transform = ofxCeres::VectorMath::createTransform(translation, rotationVector);
							MovingHead::Solution basicMovingHead{
								translation
									, rotationVector
									, transform
							};

							DistortedMovingHead::Solution distortedMovingHeadSolution{
								basicMovingHead
								, {
									panDistortionParameters[movingHeadIndex][0]
									, panDistortionParameters[movingHeadIndex][1]
									, panDistortionParameters[movingHeadIndex][2]
								}
								, {
									tiltDistortionParameters[movingHeadIndex][0]
									, tiltDistortionParameters[movingHeadIndex][1]
									, tiltDistortionParameters[movingHeadIndex][2]
								}
							};

							result.solution.movingHeads.push_back(distortedMovingHeadSolution);
						}
					}

					return result;
				}
			}
			catch (const Exception& e) {
				Result result(e);
				ofLogError("ofxCeres") << result.errorMessage;
				return result;
			}
		}
	}
}