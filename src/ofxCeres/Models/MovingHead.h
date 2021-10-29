#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

#include "../SolverSettings.h"

namespace ofxCeres {
	namespace Models {
		class MovingHead : Base {
		public:
			struct Solution {
				//parameters
				glm::vec3 translation;
				glm::vec3 rotationVector;

				//cached
				glm::mat4x4 transform;
			};

			typedef ofxCeres::Result<Solution> Result;
			static Result solve(const std::vector<glm::vec3> targetPoints
				, const std::vector<glm::vec2> panTiltValues
				, const Solution & initialSolution = Solution()
				, const SolverSettings & solverSettings = SolverSettings());
		};
	}
}