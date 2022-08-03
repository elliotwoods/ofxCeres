#pragma once

#include "../../Includes.h"

#ifdef HAS_OFXCVGUI
#include "ofxLiquidEvent.h"
#include "ofxCvGui.h"
#endif

#include <vector>
#include "ofMain.h"
#include "../../SolverSettings.h"
#include "ofxCeres/Exception.h"
#include "ofxCeres/VectorMath/VectorMath.h"

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
					bool isGlobalOrientation = true; /// e.g. Gravity direction is not affected by body rotations
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

					glm::tvec3<T> getForceError() const;

					glm::tvec3<T> getTorqueError() const;

					ofColor getColor() const
					{
						return ofColor(100);
					}

					glm::tvec3<T> getJointPosition(const string& name) const
					{
						auto vec3 = this->joints.at(name).position;
						return VectorMath::applyTransform(this->getGlobalTransformMatrix(), vec3);
					}

					glm::tvec3<T> getLoadPosition(const string& name) const
					{
						auto vec3 = this->loads.at(name).position;
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
// #include "System.inl"

#include "DrawProperties.h"

template<int jointConnectionCount, int groundSupportCount>
struct SolverError {
	SolverError(const ofxCeres::Models::StructuralAnalysis::System & system)
		: referenceSystem(system) {	}

	template<typename T>
	bool operator() (const T * const parameters
		, T * residuals) const {
		auto system = ofxCeres::Models::StructuralAnalysis::TSystem<T>(this->referenceSystem);
		system.updateStateParameters(parameters);
		
		auto & residual = residuals[0];
		residual = (T) 0;

		for (const auto & bodyIt : system.bodies) {
			residual += glm::length2(bodyIt.second->getForceError());
			residual += glm::length2(bodyIt.second->getTorqueError());
		}
		
		return true;
	}

	static ceres::CostFunction * Create(const ofxCeres::Models::StructuralAnalysis::System & referenceSystem) {
		return new ceres::AutoDiffCostFunction<SolverError, 1, (jointConnectionCount + groundSupportCount) * 3>(
			new SolverError(referenceSystem)
		);
	}

	const ofxCeres::Models::StructuralAnalysis::System & referenceSystem;
};

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			//----------
			template<typename T>
			std::string TSystem<T>::JointAddress::toString() const {
				return this->bodyName + "." + this->jointName;
			}

			//----------
			template<typename T>
			bool TSystem<T>::JointAddress::operator==(const JointAddress& other) const {
				return this->bodyName == other.bodyName && this->jointName == other.jointName;
			}

			//----------
			template<typename T>
			TSystem<T>::Body::Body() {

			}

			//----------
			template<typename T>
			glm::tvec3<T> TSystem<T>::Body::getForceError() const {
				glm::tvec3<T> total{ 0, 0, 0 }; // Local orientation frame

				auto bodyInverseRotation = glm::inverse(this->getGlobalOrientation());

				for (const auto & loadIt : this->loads) {
					auto force = loadIt.second.force;
					if (loadIt.second.isGlobalOrientation) {
						force = bodyInverseRotation * force;
					}
					total += force;
				}

				for (const auto & jointIt : this->joints) {
					total += jointIt.second.force;
				}

				return total;
			}

			//----------
			template<typename T>
			glm::tvec3<T> TSystem<T>::Body::getTorqueError() const {
				glm::tvec3<T> total{ 0, 0, 0 }; // Local orientation frame

				auto bodyInverseRotation = glm::inverse(this->getGlobalOrientation());

				for (const auto & loadIt : this->loads) {
					auto force = loadIt.second.force;
					if (loadIt.second.isGlobalOrientation) {
						force = bodyInverseRotation * force;
					}
					total += glm::cross(loadIt.second.position, force);
				}
				for (const auto & jointIt : this->joints) {
					total += glm::cross((glm::tvec3<T>) jointIt.second.position, jointIt.second.force);
				}

				return total;
			}

			//----------
			template<typename T>
			void TSystem<T>::setJointForce(const JointAddress & jointAddress
				, const glm::tvec3<T> & force
				, bool inverse) {
				auto & body = this->bodies[jointAddress.bodyName];
				auto & joint = body->joints[jointAddress.jointName];
				auto bodyInverseRotation = glm::inverse(body->getGlobalOrientation());
				joint.force = (glm::tquat<T>) bodyInverseRotation
					* force
					* (T) (inverse ? -1.0 : 1.0);
			}

			//----------
			template<int jointConnectionCount, int groundSupportCount>
			bool System::solve(const SolverSettings & solverSettings) {
				if (this->jointConnections.size() != jointConnectionCount) {
					throw(ofxCeres::Exception("Number of joint constraints does not match templated constant jointConnectionCount"));
				}
				if (this->groundSupports.size() != groundSupportCount) {
					throw(ofxCeres::Exception("Number of ground supports does not match templated constant groundSupportCount"));
				}

				this->throwIfBadJointConnection();

				double systemState[(jointConnectionCount + groundSupportCount) * 3];

				//take the state from current system
				{
					auto movingOutput = (glm::tvec3<double> *) systemState;
					for (const auto & jointConnection : this->jointConnections) {
						*movingOutput++ = (glm::tvec3<double>) jointConnection.force;
					}
					for (const auto & groundSupport : this->groundSupports) {
						*movingOutput++ = (glm::tvec3<double>) groundSupport.force;
					}
				}

				ceres::Problem problem;
				auto costFunction = SolverError<jointConnectionCount, groundSupportCount>::Create(*this);
				problem.AddResidualBlock(costFunction
					, NULL
					, systemState);

				ceres::Solver::Summary summary;
				ceres::Solve(solverSettings.options, &problem, &summary);
				
				if (solverSettings.printReport) {
					cout << summary.FullReport() << endl;
				}

				this->updateStateParameters(systemState);
				return summary.termination_type == ceres::TerminationType::CONVERGENCE;
			}
			
		}
	}
}
