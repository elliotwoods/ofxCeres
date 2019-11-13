#pragma once

#include "../../Includes.h"

#ifdef HAS_OFXCVGUI
#include "ofxLiquidEvent.h"
#endif

#include <vector>
#include "ofMain.h"
#include "../../SolverSettings.h"

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			class System;

			template<typename T>
			class TSystem {
			public:
				struct Joint {
					glm::vec3 position;
					glm::tvec3<T> force;
				};

				struct Load {
					glm::vec3 position;
					glm::vec3 force;
				};

				struct JointAddress {
					std::string bodyName;
					std::string jointName;
					std::string toString() const;
				};

				struct JointConnection {
					JointAddress A, B;
					glm::tvec3<T> force;
				};

				struct GroundSupport {
					JointAddress A;
					glm::tvec3<T> force;
				};

				class Body : public ofNode {
				public:
					Body();

					T getForceError() const;

					T getTorqueError() const;

					ofColor getColor() const {
						return ofColor(100);
					}

					std::map<std::string, Load> loads;
					std::map<std::string, Joint> joints;

#ifdef HAS_OFXCVGUI
					ofxLiquidEvent<void> onDraw;
#endif

					struct {
						ofParameter<bool> enabled{ "Enabled", true };
						ofParameter<bool> joints{ "Joints", true };
						ofParameter<bool> loads{ "Loads", true };
					} drawArgs;
				protected:
					virtual void customDraw() override {
#ifdef HAS_OFXCVGUI
						this->onDraw.notifyListeners();
#endif
					}
				};

				TSystem() {

				}

				template<typename T2>
				TSystem(const TSystem<T2> & referenceSystem) {
					for (const auto & bodyIt : referenceSystem.bodies) {
						auto newBody = make_shared<Body>();
						auto & body = bodyIt.second;

						//copy node transform
						newBody->setGlobalOrientation(body->getGlobalOrientation());
						newBody->setPosition(body->getGlobalPosition());
						newBody->setScale(body->getScale());

						for (const auto & loadIt : body->loads) {
							Load newLoad{
								loadIt.second.position
								, loadIt.second.force
							};
							newBody->loads[loadIt.first] = newLoad;
						}
						for (const auto & jointIt : body->joints) {
							Joint newJoint{
								jointIt.second.position
							};
							newBody->joints[jointIt.first] = newJoint;
						}
						this->bodies[bodyIt.first] = newBody;
					}

					for (const auto & jointConnection : referenceSystem.jointConnections) {
						JointConnection newJointConnection;
						newJointConnection.A = reinterpret_cast<const JointAddress &>(jointConnection.A);
						newJointConnection.B = reinterpret_cast<const JointAddress &>(jointConnection.B);
						this->jointConnections.push_back(newJointConnection);
					}

					for (const auto & groundSupport : referenceSystem.groundSupports) {
						GroundSupport newGroundSupport;
						newGroundSupport.A = reinterpret_cast<const JointAddress &>(groundSupport.A);
						this->groundSupports.push_back(newGroundSupport);
					}
				}

				template<typename T2>
				void updateStateParameters(const T2 * const parameters) {
					auto movingParameters = (glm::tvec3<T2> * const) parameters;
					for (auto & jointConnection : this->jointConnections) {
						jointConnection.force = (glm::tvec3<T>) *movingParameters++;
						this->setJointForce(jointConnection.A, jointConnection.force, false);
						this->setJointForce(jointConnection.B, jointConnection.force, true);
					}
					for (auto & groundSupport : this->groundSupports) {
						groundSupport.force = (glm::tvec3<T>) *movingParameters++;
						this->setJointForce(groundSupport.A, groundSupport.force, false);
					}
				}

				std::map<std::string, shared_ptr<Body>> bodies;
				std::vector<JointConnection> jointConnections;
				std::vector<GroundSupport> groundSupports;
			protected:
				void setJointForce(const JointAddress & jointAddress
					, const glm::tvec3<T> & force
					, bool inverse);
			};

			class System : public TSystem <float> {
			public:
				static ofxCeres::SolverSettings getDefaultSolverSettings();
				template<int jointConnectionCount, int groundSupportCount>
				bool solve(const SolverSettings & = getDefaultSolverSettings());
				void draw();
				void throwIfBadJointConnection() const;
			};
		}
	}
}
#include "System.inl"
