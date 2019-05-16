#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

namespace ofxCeres {
	namespace Models {
		class DistortedMovingHead : Base {
		public:
			template<typename T>
			struct TDistortion {
				T getSignal(const T & ideal) const {
					return ideal * ideal * polynomial[0]
						+ ideal * polynomial[1]
						+ polynomial[2];
				}

				T getIdeal(const T & signal) const
				{
					auto sqrt_inner = (T) 4 * polynomial[0] * signal + polynomial[1] * polynomial[1] - (T) 4 * polynomial[2] * signal;
					if (sqrt_inner < (T) 0) {
						throw(ofxCeres::Exception("No root found"));
					}

					auto sqrt_part = sqrt(sqrt_inner);

					T solution_1 = -polynomial[1] - sqrt_part;
					solution_1 /= (T) 2 * signal;

					T solution_2 = -polynomial[1] + sqrt_part;
					solution_2 /= (T) 2 * signal;

					// return solution which is closer to the original signal
					if (solution_1 == solution_1 && abs(solution_1 - signal) < abs(solution_2) - signal) {
						return solution_1;
					}
					else {
						return solution_2;
					}
				}
				T polynomial[3]; // xx x 1
			};

			typedef TDistortion<double> Distortion;

			struct Solution {
				MovingHead::Solution basicSolution;

				Distortion tiltDistortion{ {0, 1, 0} };
				Distortion panDistortion{ {0, 1, 0} };
			};

			static SolverSettings defaultSolverSettings();

			typedef ofxCeres::Result<Solution> Result;
			static Result solve(const std::vector<glm::vec3> targetPoints
				, const std::vector<glm::vec2> panTiltValuesSignal
				, const Solution & initialSolution = Solution()
				, const SolverSettings & solverSettings = DistortedMovingHead::defaultSolverSettings());
		};
	}
}