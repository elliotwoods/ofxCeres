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

			struct Constraint {
				virtual string getTypeName() const = 0;
				int markerIndex;
			};

			struct FixedMarkerConstraint : public Constraint {
				string getTypeName() const override {
					return "FixedMarkerConstraint";
				}
			};

			struct MarkerInPlaneConstraint : public Constraint {
				string getTypeName() const override {
					return "MarkerInPlaneConstraint";
				}

				glm::vec4 abcd;
			};

			typedef ofxCeres::Result<Solution> Result;
			static Result solve(const std::vector<Image>& images
				, const Solution& initialSolution
				, const std::vector<shared_ptr<Constraint>>& constraints
				, const Options& options = Options()
				, const SolverSettings& solverSettings = SolverSettings());
		};
	}
}