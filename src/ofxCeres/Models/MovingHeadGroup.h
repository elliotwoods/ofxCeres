#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

#include "../SolverSettings.h"
#include "DistortedMovingHead.h"

namespace ofxCeres {
	namespace Models {
		class MovingHeadGroup : Base {
		public:
			struct Solution {
				std::vector<glm::vec3> markerPositions;
				std::vector<DistortedMovingHead::Solution> movingHeads;
			};

			// The view from a single moving head
			struct Image {
				std::vector<glm::vec2> panTiltSignal;
				std::vector<size_t> markerIndex;
			};

			struct Options {
				bool noDistortion;
			};

			typedef ofxCeres::Result<Solution> Result;
			static Result solve(const std::vector<Image>& images
				, const Solution& initialSolution
				, const std::vector<bool>& fixMarkerPositions
				, const Options& options = Options()
				, const SolverSettings& solverSettings = SolverSettings());
		};
	}
}