#pragma once

#include "ofxCeres.h"
#include "ofxLiquidEvent.h"
#include <vector>
#include <glm/glm.hpp>

namespace Data {
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

			T getForceError() const {
				glm::tvec3<T> total;

				for (const auto & loadIt : this->loads) {
					total += loadIt.second.force;
				}
				for (const auto & jointIt : this->joints) {
					total += jointIt.second.force;
				}

				return ofxCeres::VectorMath::dot(total, total);
			}

			T getTorqueError() const {
				glm::tvec3<T> total;

				for (const auto & loadIt : this->loads) {
					total += glm::cross(loadIt.second.position, loadIt.second.force);
				}
				for (const auto & jointIt : this->joints) {
					total += ofxCeres::VectorMath::cross((glm::tvec3<T>) jointIt.second.position, jointIt.second.force);
				}

				return ofxCeres::VectorMath::dot(total, total);
			}

			ofColor getColor() const {
				return ofColor(100);
			}

			std::map<std::string, Load> loads;
			std::map<std::string, Joint> joints;

			ofxLiquidEvent<void> onDraw;
			struct {
				ofParameter<bool> enabled{ "Enabled", true };
				ofParameter<bool> joints{ "Joints", true };
				ofParameter<bool> loads { "Loads", true };
			} drawArgs;
		protected:
			void customDraw() override {
				this->onDraw.notifyListeners();
			}
		};

		TSystem() {

		}

		template<typename T2>
		TSystem(const TSystem<T2> & referenceSystem) {
			for (const auto & bodyIt : referenceSystem.bodies) {
				Body newBody;
				auto & body = bodyIt.second;
				for (const auto & loadIt : body.loads) {
					Load newLoad{
						loadIt.second.position
						, loadIt.second.force
					};
					newBody.loads[loadIt.first] = newLoad;
				}
				for (const auto & jointIt : body.joints) {
					Joint newJoint{
						jointIt.second.position
					};
					newBody.joints[jointIt.first] = newJoint;
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
				this->bodies[jointConnection.A.bodyName].joints[jointConnection.A.jointName].force = jointConnection.force;
				this->bodies[jointConnection.B.bodyName].joints[jointConnection.B.jointName].force = -jointConnection.force;
			}
			for (auto & groundSupport : this->groundSupports) {
				groundSupport.force = (glm::tvec3<T>) *movingParameters++;
				this->bodies[groundSupport.A.bodyName].joints[groundSupport.A.jointName].force = groundSupport.force;
			}
		}

		std::map<std::string, Body> bodies;
		std::vector<JointConnection> jointConnections;
		std::vector<GroundSupport> groundSupports;
	};

	class System : public TSystem <float> {
	public:
		template<int jointConnectionCount, int groundSupportCount>
		void solve();
		void draw();
		void populateInspector(shared_ptr<ofxCvGui::Panels::Widgets>);
	};
}

#include "System.inl"
