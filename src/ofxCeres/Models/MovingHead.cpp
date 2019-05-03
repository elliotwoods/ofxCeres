#include "../VectorMath/VectorMath.h"

#include "MovingHead.h"

#include "ofLog.h"
#include <ceres/ceres.h>

using namespace std;

struct MovingHeadError {
	MovingHeadError(const glm::tvec3<double> & targetPoint, const glm::tvec2<double> & panTiltValues)
		: targetPoint(targetPoint)
		, panTiltValues(panTiltValues) {}

	template <typename T>
	bool operator()(const T * const transformParameters
		, T * residuals) const {

		//--
		//Extract parameters
		glm::tvec3<T> translation(transformParameters[0], transformParameters[1], transformParameters[2]);
		glm::tvec3<T> rotationVector(transformParameters[3], transformParameters[4], transformParameters[5]);

		T tiltOffset = transformParameters[6];

		auto transform = ofxCeres::VectorMath::createTransform(translation, rotationVector);
		//
		//--


		//--
		// Ignore point if it is in same position as the light fixture right now
		if ((glm::tvec3<T>)this->targetPoint == translation) {
			residuals[0] = (T) 0.0;
			return true;
		}
		//
		//--
			

		//--
		//World -> Object space
		//

		//apply rigid body transform
		glm::tvec4<T> targetInViewSpace4 = glm::inverse(transform) * glm::tvec4<T>(this->targetPoint, 1.0);
		targetInViewSpace4 /= targetInViewSpace4.w;
		auto targetInViewSpace = glm::tvec3<T>(targetInViewSpace4);

		//
		//--

		glm::tvec3<T> rayCastForPanTiltValues = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt<T>(this->panTiltValues, tiltOffset);

		//--
		//Get the disparity between the real and actual object space rays
		//
		auto dotProduct = ofxCeres::VectorMath::dot(rayCastForPanTiltValues, targetInViewSpace);
		dotProduct = dotProduct / (ofxCeres::VectorMath::length(rayCastForPanTiltValues) * ofxCeres::VectorMath::length(targetInViewSpace));
		auto angleBetweenResults = acos(dotProduct);

		residuals[0] = angleBetweenResults * angleBetweenResults;
		
		return true;
	}

	static ceres::CostFunction * Create(const glm::tvec3<double> & targetPoint, const glm::tvec2<double> & panTiltValues) {
		return new ceres::AutoDiffCostFunction<MovingHeadError, 1, 7>(
			new MovingHeadError(targetPoint, panTiltValues)
			);
	}

	const glm::tvec3<double> targetPoint;
	const glm::tvec2<double> panTiltValues;
};

struct MovingHeadBias {
	template <typename T>
	bool operator()(const T * const transformParameters
		, T * residuals) const {
		T tiltOffset = transformParameters[6];
		
		residuals[0] = tiltOffset * tiltOffset * (T) 1e-6;

		return true;
	}

	static ceres::CostFunction * Create() {
		return new ceres::AutoDiffCostFunction<MovingHeadBias, 1, 7>(
			new MovingHeadBias()
			);
	}
};

namespace ofxCeres {
	namespace Models {
		//----------
		MovingHead::Result MovingHead::solve(const std::vector<glm::vec3> targetPoints
			, const std::vector<glm::vec2> panTiltValues
			, const Solution & initialSolution) {
			try {
				if (targetPoints.size() != panTiltValues.size()) {
					throw(ofxCeres::Exception("targetPoints.size() != panTiltValues.size()"));
				}

				// Initialize parameters
				double parameters[7] = {
					initialSolution.translation[0]
					, initialSolution.translation[1]
					, initialSolution.translation[2]
					, initialSolution.rotationVector[0]
					, initialSolution.rotationVector[1]
					, initialSolution.rotationVector[2]
					, initialSolution.tiltOffset
				};

				ceres::Problem problem;
				size_t size = targetPoints.size();
				for (size_t i = 0; i < size; i++) {
					ceres::CostFunction * costFunction = MovingHeadError::Create(targetPoints[i], panTiltValues[i]);
					problem.AddResidualBlock(costFunction
						, NULL
						, parameters);
				}
				problem.AddResidualBlock(MovingHeadBias::Create()
					, NULL
					, parameters);

				ceres::Solver::Options options;
				options.linear_solver_type = ceres::DENSE_QR;
				
				options.minimizer_progress_to_stdout = true;
				options.max_num_iterations = 100;
				options.function_tolerance = 5e-6;
				ceres::Solver::Summary summary;
				ceres::Solve(options, &problem, &summary);
				std::cout << summary.FullReport() << "\n";

				// construct result
				{
					glm::vec3 translation(parameters[0], parameters[1], parameters[2]);
					glm::vec3 rotationVector(parameters[3], parameters[4], parameters[5]);

					auto tiltOffset = (float)parameters[6];

					Solution solution{
						translation
						, rotationVector
						, tiltOffset
						, ofxCeres::VectorMath::createTransform(translation, rotationVector)
					};

					Result result{
						true
						, solution
						, sqrt(summary.final_cost / (double)size)
						, summary
					};
					return result;
				}
			}
			catch (const Exception & e) {
				Result result;
				result.success = false;
				result.errorMessage = string(e.what());
				ofLogError("ofxCeres") << result.errorMessage;

				return result;
			}
		}
	}
}