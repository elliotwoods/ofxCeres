#include "pch_ofxCeres.h"
#include "../VectorMath/VectorMath.h"

#include "MovingHead.h"
#include "DistortedMovingHead.h"

#include "ofLog.h"

using namespace std;

#include "DistortedMovingHeadError.h"

struct DistortionBias {
	template<typename T>
	bool operator()(const T * const distortionParameters
		, T * residuals) const {
		residuals[0] = distortionParameters[0] * distortionParameters[0] / (T) 100.0;
		return true;
	}

	static ceres::CostFunction * Create() {
		return new ceres::AutoDiffCostFunction<DistortionBias, 1, 3>(
			new DistortionBias()
		);
	}
};

namespace ofxCeres {
	namespace Models {
		//----------
		SolverSettings DistortedMovingHead::defaultSolverSettings() {
			SolverSettings solverSettings;

			// if we want to add some specific options for this solver, do so here
			solverSettings.options.max_num_iterations = 1000;

			return solverSettings;
		}

		//----------
		DistortedMovingHead::Result DistortedMovingHead::solve(const std::vector<glm::vec3> targetPoints
			, const std::vector<glm::vec2> panTiltValues
			, const Solution & initialSolution
			, const SolverSettings & solverSettings) {
			try {
				if (targetPoints.size() != panTiltValues.size()) {
					throw(ofxCeres::Exception("targetPoints.size() != panTiltValues.size()"));
				}

				//--
				// Initialize parameters
				//--
				//
				double basicParameters[7] = {
					initialSolution.basicSolution.translation[0]
					, initialSolution.basicSolution.translation[1]
					, initialSolution.basicSolution.translation[2]
					, initialSolution.basicSolution.rotationVector[0]
					, initialSolution.basicSolution.rotationVector[1]
					, initialSolution.basicSolution.rotationVector[2]
				};

				double panDistortionParameters[3];
				double tiltDistortionParameters[3];
				for (int i = 0; i < 3; i++) {
					panDistortionParameters[i] = initialSolution.panDistortion[i];
					tiltDistortionParameters[i] = initialSolution.tiltDistortion[i];
				}
				//
				//--


				//--
				// Build the problem
				//--
				//
				ceres::Problem problem;
				size_t size = targetPoints.size();
				for (size_t i = 0; i < size; i++) {
					ceres::CostFunction * costFunction = DistortedMovingHeadError::Create(targetPoints[i], panTiltValues[i]);
					problem.AddResidualBlock(costFunction
						, NULL
						, basicParameters
						, panDistortionParameters
						, tiltDistortionParameters);
				}
				//
				//--

				//--
				// Solve
				//--
				//
				ceres::Solver::Summary summary;
				ceres::Solve(solverSettings.options, &problem, &summary);

				if (solverSettings.printReport) {
					std::cout << summary.FullReport() << "\n";
				}
				//
				//--



				// construct result
				{
					glm::vec3 translation(basicParameters[0], basicParameters[1], basicParameters[2]);
					glm::vec3 rotationVector(basicParameters[3], basicParameters[4], basicParameters[5]);

					Result result(summary, sqrt(summary.final_cost / (double)size));

					result.solution = {
						{
							translation
							, rotationVector
							, ofxCeres::VectorMath::createTransform(translation, rotationVector)
						}
						, {
							panDistortionParameters[0]
							, panDistortionParameters[1]
							, panDistortionParameters[2]
						}
						, {
							tiltDistortionParameters[0]
							, tiltDistortionParameters[1]
							, tiltDistortionParameters[2]
						}
					};

					return result;
				}
			}
			catch (const Exception & e) {
				Result result(e);
				ofLogError("ofxCeres") << result.errorMessage;
				return result;
			}
		}
	}
}