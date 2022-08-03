#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

#include "../SolverSettings.h"

#include "pch_ofxCeres.h"
#include "ofxCeres/VectorMath/VectorMath.h"

template<size_t Order>
struct PolyFitError {
	PolyFitError(const double& x, const double& y)
		: x(x)
		, y(y){}

	template <typename T>
	bool operator()(const T* const coefficients
		, T* residuals) const {

		T result = (T)0;
		double xx = 1;
		for (size_t i = 0; i < Order; i++) {
			result += xx * coefficients[i];
			xx *= x;
		}
		residuals[0] = result - this->y;
		return true;
	}

	static ceres::CostFunction* Create(const double& x, const double& y) {
		return new ceres::AutoDiffCostFunction<PolyFitError<Order>, 1, Order>(
			new PolyFitError(x, y)
			);
	}

	const double x;
	const double y;
};

namespace ofxCeres {
	namespace Models {
		template<size_t Order>
		class PolyFit : Base {
		public:
			struct Solution {
				Solution() {
					// Initialise for identity
					for (size_t i = 0; i < Order; i++) {
						this->coefficients[i] = i == 1 ? 1 : 0;
					}
				}

				double operator()(const double& x) {
					double result = 0.0;
					double xx = 1.0;
					for (size_t i = 0; i < Order; i++) {
						result += xx * this->coefficients[i];
						xx *= x;
					}
					return result;
				}

				array<double, Order> coefficients;
			};

			typedef ofxCeres::Result<Solution> Result;
			static Result solve(const std::vector<float>& x
				, const std::vector<float>& y
				, const Solution& initialSolution = Solution()
				, const SolverSettings& solverSettings = SolverSettings()) {
				try {
					// Check some errors
					if (x.size() != y.size()) {
						throw(ofxCeres::Exception("x.size() != y.size()"));
					}
					auto size = x.size();

					// Make a copy of the solution
					auto solution = initialSolution;

					// Build up the problem, cost functions
					ceres::Problem problem;
					for (size_t i = 0; i < size; i++) {
						auto costFunction = PolyFitError<Order>::Create(x[i], y[i]);
						problem.AddResidualBlock(costFunction
							, NULL
							, solution.coefficients.data()
						);
					}

					ceres::Solver::Summary summary;
					ceres::Solve(solverSettings.options, &problem, &summary);

					if (solverSettings.printReport) {
						std::cout << summary.FullReport() << "\n";
					}

					Result result(summary, sqrt(summary.final_cost / (double)size));
					result.solution = solution;
					return result;
				}
				catch (const Exception& e) {
					Result result(e);
					ofLogError("ofxCeres") << result.errorMessage;
					return result;
				}
			}
		};
	}
}

