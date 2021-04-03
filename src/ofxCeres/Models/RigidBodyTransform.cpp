#include "pch_ofxCeres.h"
#include "../VectorMath/VectorMath.h"

#include "RigidBodyTransform.h"

#include "ofLog.h"
#include <ceres/ceres.h>

using namespace std;

struct RigidBodyTransformError {
	RigidBodyTransformError(const glm::tvec3<double> & untransformedPoint, const glm::tvec3<double> & transformedPoint)
		: untransformedPoint(untransformedPoint)
		, transformedPoint(transformedPoint) {}

	template <typename T>
	bool operator()(const T * const transformParameters
		, T * residuals) const {

		glm::tvec3<T> translation(transformParameters[0], transformParameters[1], transformParameters[2]);
		glm::tvec3<T> rotationVector(transformParameters[3], transformParameters[4], transformParameters[5]);

		auto transform = ofxCeres::VectorMath::createTransform(translation, rotationVector);
		auto predictedTransformedPoint = transform * glm::tvec4<T>(this->untransformedPoint, 1.0);

		predictedTransformedPoint /= predictedTransformedPoint.w;

		for (int i = 0; i < 3; i++) {
			residuals[i] = this->transformedPoint[i] - predictedTransformedPoint[i];
		}

		return true;
	}

	static ceres::CostFunction * Create(const glm::tvec3<double> & untransformedPoint, const glm::tvec3<double> & transformedPoint) {
		return (new ceres::AutoDiffCostFunction<RigidBodyTransformError, 3, 6>(
			new RigidBodyTransformError(untransformedPoint, transformedPoint)));
	}

	glm::tvec3<double> untransformedPoint;
	glm::tvec3<double> transformedPoint;
};

namespace ofxCeres {
	namespace Models {
		//----------
		RigidBodyTransform::Result RigidBodyTransform::solve(const std::vector<glm::vec3> untransformedPoints
			, const std::vector<glm::vec3> transformedPoints) {
			try {
				if (untransformedPoints.size() != transformedPoints.size()) {
					throw(ofxCeres::Exception("untransformedPoints.size() != transformedPoints.size()"));
				}


				double parameters[6];
				for (auto & parameter : parameters) {
					parameter = 0.0;
				}

				ceres::Problem problem;
				size_t size = untransformedPoints.size();
				for (size_t i = 0; i < size; i++) {
					ceres::CostFunction * costFunction = RigidBodyTransformError::Create(untransformedPoints[i], transformedPoints[i]);
					problem.AddResidualBlock(costFunction
						, NULL
						, parameters);
				}

				ceres::Solver::Options options;
				options.linear_solver_type = ceres::DENSE_SCHUR;
				options.minimizer_progress_to_stdout = true;
				ceres::Solver::Summary summary;
				ceres::Solve(options, &problem, &summary);
				std::cout << summary.FullReport() << "\n";

				// construct result
				{
					glm::vec3 translation(parameters[0], parameters[1], parameters[2]);
					glm::vec3 rotationVector(parameters[3], parameters[4], parameters[5]);

					Solution solution{
						translation
						, rotationVector
						, ofxCeres::VectorMath::createTransform(translation, rotationVector)
					};

					Result result(summary);
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