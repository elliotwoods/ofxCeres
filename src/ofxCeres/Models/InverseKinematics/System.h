#pragma once

#include "ofxCeres/SolverSettings.h"
#include <vector>
#include <glm/glm.hpp>

namespace ofxCeres {
	namespace Models {
		namespace InverseKinematics {
			template<typename T>
			struct TPositionAndRotation {
				glm::tvec2<T> position;
				T rotation;
			};
			typedef TPositionAndRotation<double> PositionAndRotation;

			class System {
			public:
				struct JointPositionContraint {
					size_t jointIndex;
					glm::vec2 position;
					float weight = 1.0f;

					template<typename T>
					T getDistance(TPositionAndRotation<T> * const PandRs) const {
						auto distance = ofxCeres::VectorMath::distance2(PandRs[this->jointIndex].position
							, (glm::tvec2<T>) this->position);
						return distance * (T)this->weight;
					}
				};

				struct JointAngleConstraint {
					size_t jointIndex;
					float rotation;
					float weight = 1.0f;

					template<typename T>
					T getDistance(TPositionAndRotation<T> * const PandRs) const {
						auto distance = PandRs[this->jointIndex].rotation - (T)this->rotation;
						return distance * distance * (T)this->weight;
					}
				};

				template<typename T>
				void getPandRsForState(const T * const state, TPositionAndRotation<T> * output) const {
					glm::tvec2<T> currentPosition(0, 0);
					auto currentRotation = (T) 0.0;

					auto movingOutput = output;

					// first point
					{
						movingOutput->position = currentPosition;
						movingOutput->rotation = currentRotation;
						movingOutput++;
					}

					for (size_t i = 0; i < this->bodyLengths.size(); i++) {
						currentRotation += state[i];
						T dx = cos(currentRotation) * (T)this->bodyLengths[i];
						T dy = sin(currentRotation) * (T)this->bodyLengths[i];
						currentPosition += glm::tvec2<T>(dx, dy);

						movingOutput->position = currentPosition;
						movingOutput->rotation = currentRotation;
						movingOutput++;
					}
				}

				std::vector<float> bodyLengths;
				std::vector<double> currentRotationState;
				std::vector<JointPositionContraint> jointPositionConstraints;
				std::vector<JointAngleConstraint> jointAngleConstraints;

				template<typename T>
				T getDistance(T * const state) const {
					auto distance = (T) 0.0;
					for (const auto & jointPositionConstraint : this->jointPositionConstraints) {
						distance += jointPositionConstraint.getDistance(state);
					}
					for (const auto & jointAngleConstraint : this->jointAngleConstraints) {
						distance += jointAngleConstraint.getDistance(state);
					}
					return distance;
				}

				static ofxCeres::SolverSettings getDefaultSolverSettings();

				template<int stateCount>
				bool solve(const SolverSettings & = getDefaultSolverSettings());

				vector<PositionAndRotation> getCurrentPositionsAndRotations() const;
				void draw();

				struct {
					int maxIterations = 10;
					float noise = 1e-5;
					float finalCostTarget = 1e-7;
				} annealing;
			};
		}
	}
}

#include "System.inl"