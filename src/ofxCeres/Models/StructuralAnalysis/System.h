#pragma once

#include "../../Includes.h"

#ifdef HAS_OFXCVGUI
#include "ofxLiquidEvent.h"
#include "ofxCvGui.h"
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
					bool operator==(const JointAddress&) const;
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

					template<typename T2>
					Body(const T2& otherBody)
					{
						//copy node transform
						this->setGlobalOrientation(otherBody.getGlobalOrientation());
						this->setPosition(otherBody.getGlobalPosition());
						this->setScale(otherBody.getScale());

						for (const auto& loadIt : otherBody.loads) {
							Load newLoad{
								loadIt.second.position
								, loadIt.second.force
							};
							this->loads[loadIt.first] = newLoad;
						}
						for (const auto& jointIt : otherBody.joints) {
							Joint newJoint{
								jointIt.second.position
								// We don't copy force since this is re-applied using parameters
							};
							this->joints[jointIt.first] = newJoint;
						}
					}

					T getForceError() const;

					T getTorqueError() const;

					ofColor getColor() const
					{
						return ofColor(100);
					}

					glm::tvec3<T> getJointPosition(const string& name) const
					{
						auto vec3 = this->joints.at(name).position;
						return VectorMath::applyTransform(this->getGlobalTransformMatrix(), vec3);
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
						const auto & body = bodyIt.second;
						auto newBody = make_shared<Body>(*body);
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

				JointAddress findConnectedJoint(JointAddress jointAddress)
				{
					for (const auto& jointConnection : this->jointConnections)
					{
						if (jointConnection.A == jointAddress)
						{
							return jointConnection.B;
						}
						if (jointConnection.B == jointAddress)
						{
							return jointConnection.A;
						}
					}
					throw(Exception("No joint found connected to " + jointAddress.toString()));
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

#ifdef HAS_OFXCVGUI
				void populateInspector(shared_ptr<ofxCvGui::Panels::Widgets>);
#endif
			};
		}
	}
}
#include "System.inl"
