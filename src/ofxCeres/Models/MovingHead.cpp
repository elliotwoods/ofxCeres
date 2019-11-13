#include "pch_ofxCeres.h"
#include "../VectorMath/VectorMath.h"

#include "MovingHead.h"

#include "ofLog.h"
#include <ceres/ceres.h>

#include "MovingHeadError.h"

using namespace std;

namespace ofxCeres {
	namespace Models {
		//----------
		MovingHead::Result MovingHead::solve(const std::vector<glm::vec3> targetPoints
			, const std::vector<glm::vec2> panTiltValues
			, const Solution & initialSolution
			, const SolverSettings & solverSettings) {
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

				ceres::Solver::Summary summary;
				ceres::Solve(solverSettings.options, &problem, &summary);

				if (solverSettings.printReport) {
					std::cout << summary.FullReport() << "\n";
				}

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