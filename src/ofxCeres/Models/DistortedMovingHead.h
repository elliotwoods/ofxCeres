#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>
#include "MovingHead.h"

#define OFXCERES_DISTORTEDMOVINGHEAD_PARAMETER_COUNT 3

namespace ofxCeres {
	namespace Models {
		class DistortedMovingHead : Base {
		public:
			struct Solution {
				MovingHead::Solution basicSolution;

				double panDistortion[OFXCERES_DISTORTEDMOVINGHEAD_PARAMETER_COUNT]{ 0, 1, 0 };
				double tiltDistortion[OFXCERES_DISTORTEDMOVINGHEAD_PARAMETER_COUNT]{ 0, 1, 0 };
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