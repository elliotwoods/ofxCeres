#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

namespace ofxCeres {
	namespace Models {
		class DistortedMovingHead : Base {
		public:
			struct Solution {
				MovingHead::Solution basicSolution;

				double panDistortion[3]{ 0, 1, 0 };
				double tiltDistortion[3]{ 0, 1, 0 };
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