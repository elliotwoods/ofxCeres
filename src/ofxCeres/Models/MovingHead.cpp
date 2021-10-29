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
				double translationParameters[3] = {
					initialSolution.translation[0]
					, initialSolution.translation[1]
					, initialSolution.translation[2]
				};
				double rotationParameters[3] = {
					initialSolution.rotationVector[0]
					, initialSolution.rotationVector[1]
					, initialSolution.rotationVector[2]
				};

				ceres::Problem problem;
				size_t size = targetPoints.size();
				for (size_t i = 0; i < size; i++) {
					ceres::CostFunction * costFunction = MovingHeadError::Create(targetPoints[i]
						, panTiltValues[i]);
					problem.AddResidualBlock(costFunction
						, NULL
						, translationParameters
						, rotationParameters);
				}

				ceres::Solver::Summary summary;
				ceres::Solve(solverSettings.options, &problem, &summary);

				if (solverSettings.printReport) {
					std::cout << summary.FullReport() << "\n";
				}

				// construct result
				{
					glm::vec3 translation(translationParameters[0], translationParameters[1], translationParameters[2]);
					glm::vec3 rotationVector(rotationParameters[0], rotationParameters[1], rotationParameters[2]);

					Result result(summary, sqrt(summary.final_cost / (double)size));

					result.solution = {
						translation
						, rotationVector
						, ofxCeres::VectorMath::createTransform(translation, rotationVector)
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