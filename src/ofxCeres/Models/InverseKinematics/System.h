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
					T getDistance(const vector<TPositionAndRotation<T>> & PandRs) const {
						auto distance = glm::distance2(PandRs[this->jointIndex].position
							, (glm::tvec2<T>) this->position);
						return distance * (T)this->weight;
					}
				};

				struct JointAngleConstraint {
					size_t jointIndex;
					float rotation;
					float weight = 1.0f;

					template<typename T>
					T getDistance(const vector<TPositionAndRotation<T>> & PandRs) const {
						auto distance = PandRs[this->jointIndex].rotation - (T)this->rotation;
						return distance * distance * (T)this->weight;
					}
				};

				struct DomainConstraint {
					ofRectangle domain;
					float weight = 1.0f;

					template<typename T>
					T getDistance(const vector<TPositionAndRotation<T>> & PandRs) const {
						auto cost = (T) 0.0f;

						for (const auto & it : PandRs) {
							auto position = it.position;
							if (position.x < (T) this->domain.x) {
								cost += (T) this->domain.x - position.x;
							}
							if (position.y < (T) this->domain.y) {
								cost += (T) this->domain.y - position.y;
							}
							if (position.x > (T) (this->domain.x + this->domain.width)) {
								cost += position.x - (T) (this->domain.x + this->domain.width);
							}
							if (position.y > (T) (this->domain.y + this->domain.height)) {
								cost += position.y - (T) (this->domain.y + this->domain.height);
							}
						}
					
						return cost * cost * (T)this->weight;
					}
				};

				template<typename T>
				vector<TPositionAndRotation<T>> getPandRsForState(const T * const state) const {
					vector<TPositionAndRotation<T>> result;

					glm::tvec2<T> currentPosition(0, 0);
					auto currentRotation = (T) 0.0;

					// first point
					{
						result.push_back({ currentPosition, currentRotation });
					}

					for (size_t i = 0; i < this->bodyLengths.size(); i++) {
						currentRotation += state[i];
						T dx = cos(currentRotation) * (T)this->bodyLengths[i];
						T dy = sin(currentRotation) * (T)this->bodyLengths[i];
						currentPosition += glm::tvec2<T>(dx, dy);

						result.push_back({ currentPosition, currentRotation });
					}

					return result;
				}

				std::vector<float> bodyLengths;
				std::vector<double> currentRotationState;
				std::vector<JointPositionContraint> jointPositionConstraints;
				std::vector<JointAngleConstraint> jointAngleConstraints;
				std::vector<DomainConstraint> domainConstraints;

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
#include "pch_ofxCeres.h"
#include "ofPolyline.h"
#include "ofxCeres/Exception.h"

template<int stateCount>
struct IKSolverError {
    IKSolverError(const ofxCeres::Models::InverseKinematics::System & system)
        : system(system) {

    }

    template<typename T>
    bool operator()(const T * const parameters
        , T * residuals) const {
        auto & residual = residuals[0];
        residual = (T) 0.0;

        auto PandRs = system.getPandRsForState<T>(parameters);
        for (const auto & jointPositionConstraint : this->system.jointPositionConstraints) {
            residual += jointPositionConstraint.getDistance(PandRs);
        }
        for (const auto & jointAngleConstraint: this->system.jointAngleConstraints) {
            residual += jointAngleConstraint.getDistance(PandRs);
        }
        for (const auto & domainConstraint : this->system.domainConstraints) {
            residual += domainConstraint.getDistance(PandRs);
        }
        return true;
    }

    static ceres::CostFunction * Create(const ofxCeres::Models::InverseKinematics::System & system) {
        return new ceres::AutoDiffCostFunction<IKSolverError, 1, stateCount>(
            new IKSolverError(system)
        );
    }

    const ofxCeres::Models::InverseKinematics::System & system;
};

namespace ofxCeres {
    namespace Models {
        namespace InverseKinematics {
            //-----------
            template<int stateCount>
            bool System::solve(const SolverSettings & solverSettings) {
                //check sizes
                if (stateCount > this->bodyLengths.size()) {
                    throw(ofxCeres::Exception("bodyLengths doesn't match hard-coded stateCount"));
                }

                //initialise the parameters if required
                if (this->currentRotationState.size() != stateCount) {
                    this->currentRotationState.assign(stateCount, 0.0);
                }

                ceres::Problem problem;
                auto costFunction = IKSolverError<stateCount>::Create(*this);
                problem.AddResidualBlock(costFunction
                    , NULL
                    , this->currentRotationState.data());

                ceres::Solver::Summary summary;

                //Annealing adds noise if we solved but dont meet the threshold
                for (int i = 0; i < this->annealing.maxIterations; i++) {
                    ceres::Solve(solverSettings.options, &problem, &summary);
                    if (summary.termination_type == ceres::CONVERGENCE) {
                        if (summary.final_cost < this->annealing.finalCostTarget) {
                            if (solverSettings.printReport) {
                                cout << summary.FullReport() << endl;
                            }
                            return true;
                        }
                        else {
                            for (int i = 0; i < stateCount; i++) {
                                this->currentRotationState[i] += ofRandomf() * this->annealing.noise;
                            }
                        }
                    }
                }

                if (solverSettings.printReport) {
                    cout << summary.FullReport() << endl;
                }
                return false;
            }
        }
    }
}

//#include "System.inl"
// #include "pch_ofxCeres.h"
// #include "ofPolyline.h"
// #include "ofxCeres/Exception.h"

// template<int stateCount> 
// struct IKSolverError {
// 	IKSolverError(const ofxCeres::Models::InverseKinematics::System & system)
// 		: system(system) {

// 	}

// 	template<typename T>
// 	bool operator()(const T * const parameters
// 		, T * residuals) const {
// 		auto & residual = residuals[0];
// 		residual = (T) 0.0;

// 		auto PandRs = system.getPandRsForState<T>(parameters);
// 		for (const auto & jointPositionConstraint : this->system.jointPositionConstraints) {
// 			residual += jointPositionConstraint.getDistance(PandRs);
// 		}
// 		for (const auto & jointAngleConstraint: this->system.jointAngleConstraints) {
// 			residual += jointAngleConstraint.getDistance(PandRs);
// 		}
// 		for (const auto & domainConstraint : this->system.domainConstraints) {
// 			residual += domainConstraint.getDistance(PandRs);
// 		}
// 		return true;
// 	}

// 	static ceres::CostFunction * Create(const ofxCeres::Models::InverseKinematics::System & system) {
// 		return new ceres::AutoDiffCostFunction<IKSolverError, 1, stateCount>(
// 			new IKSolverError(system)
// 		);
// 	}

// 	const ofxCeres::Models::InverseKinematics::System & system;
// };

// namespace ofxCeres {
// 	namespace Models {
// 		namespace InverseKinematics {
// 			//-----------
// 			template<int stateCount>
// 			bool System::solve(const SolverSettings & solverSettings) {
// 				//check sizes
// 				if (stateCount > this->bodyLengths.size()) {
// 					throw(ofxCeres::Exception("bodyLengths doesn't match hard-coded stateCount"));
// 				}

// 				//initialise the parameters if required
// 				if (this->currentRotationState.size() != stateCount) {
// 					this->currentRotationState.assign(stateCount, 0.0);
// 				}

// 				ceres::Problem problem;
// 				auto costFunction = IKSolverError<stateCount>::Create(*this);
// 				problem.AddResidualBlock(costFunction
// 					, NULL
// 					, this->currentRotationState.data());

// 				ceres::Solver::Summary summary;

// 				//Annealing adds noise if we solved but dont meet the threshold
// 				for (int i = 0; i < this->annealing.maxIterations; i++) {
// 					ceres::Solve(solverSettings.options, &problem, &summary);
// 					if (summary.termination_type == ceres::CONVERGENCE) {
// 						if (summary.final_cost < this->annealing.finalCostTarget) {
// 							if (solverSettings.printReport) {
// 								cout << summary.FullReport() << endl;
// 							}
// 							return true;
// 						}
// 						else {
// 							for (int i = 0; i < stateCount; i++) {
// 								this->currentRotationState[i] += ofRandomf() * this->annealing.noise;
// 							}
// 						}
// 					}
// 				}

// 				if (solverSettings.printReport) {
// 					cout << summary.FullReport() << endl;
// 				}
// 				return false;
// 			}
// 		}
// 	}
// }
