#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

namespace ofxCeres {
	namespace Models {
		class RigidBodyTransform : Base {
		public:
			struct Solution {
				//parameters
				glm::vec3 translation;
				glm::vec3 rotationVector;

				//cached
				glm::mat4x4 transform;
			};

			typedef ofxCeres::Result<Solution> Result;
			static Result solve(const std::vector<glm::vec3> untransformedPoints
				, const std::vector<glm::vec3> transformedPoints);
		};
	}
}