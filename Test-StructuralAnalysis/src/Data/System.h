#pragma once

#include "ofxCeres.h"
#include <vector>
#include <glm/glm.hpp>

#define TEMPLATE_HEAD template<int jointConnectionCount, int groundSupportCount, typename T>
#define TEMPLATE_BODY <jointConnectionCount, groundSupportCount, T>

namespace Data {
	template<int jointConnectionCount, int groundSupportCount>
	class System;

	TEMPLATE_HEAD
	class TSystem {
	public:
		struct Joint {
			glm::vec3 position;
			glm::tvec3<T> force;
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

		struct Load {
			glm::tvec3<T> postiion;
			glm::tvec3<T> force;
		};

		class Body {
		public:
			T getForceError() {
				tvec3<T> total;

				for (const auto & load : this->loads) {
					total += load.force;
				}
				for (const auto & joint : this->joint) {
					total += joint.force;
				}

				return glm::dot(total, total);
			}

			T getTorqueError() {
				tvec3<T> total;

				for (const auto & load : this->loads) {
					total += glm::cross(load.position, load.force);
				}
				for (const auto & joint : this->joint) {
					total += glm::cross(joint.position, joint.force);
				}

				return glm::dot(total, total);
			}

			std::map<std::string, Load> loads;
			std::map<std::string, Joint> joints;
		};

		TSystem() {

		}

		template<typename T2>
		TSystem(const TSystem<jointConnectionCount, groundSupportCount, T2> & referenceSystem) {
			for (const auto & bodyIt : referenceSystem.bodies) {
				Body newBody;
				auto & body = bodyIt.second;
				for (const auto & loadIt : body.loads) {
					Load newLoad{
						(glm::tvec3<T>) loadIt.second.force
						, (glm::tvec3<T>) loadIt.second.position
					};
					newBody.loads[loadIt.first] = newLoad;
				}
				for (const auto & jointIt : body.joints) {
					Load newJoint{
						(glm::tvec3<T>) jointIt.second.position
					};
					newBody.joints[jointIt.first] = newJoint;
				}
				this->bodies[bodyIt.first] = newBody;
			}

			for (const auto & jointConnection : referenceSystem.jointConnections) {
				JointConnection newJointConnection;
				newJointConnection.A = jointConnection.A;
				newJointConnection.B = jointConnection.B;
				
				this->jointConnections.push_back(newJointConnection);
			}

			for (const auto & groundSupport : referenceSystem.groundSupports) {
				GroundSupport newGroundSupport;

				newGroundSupport.A = groundSupport.A;

				this->groundSupports.push_back(newGroundSupport);
			}
		}

		template<typename T2>
		void updateStateParameters(const T2 * const parameters) {
			auto movingParameters = (glm::tvec3<T2> * const) parameters;
			for (const auto & jointConnection : this->jointConnections) {
				jointConnection.force = (glm::tvec3<T>) *movingParameters++;
				this->bodies[jointConnection.A.bodyName].joints[jointConnection.A.jointName] = jointConnection.force;
				this->bodies[jointConnection.B.bodyName].joints[jointConnection.B.jointName] = -jointConnection.force;
			}
			for (const auto & groundSupport : this->groundSupports) {
				groundSupport.force = (glm::tvec3<T>) *movingParameters++;
				this->bodies[groundSupport.A.bodyName].joints[groundSupport.A.jointName] = groundSupport.force;
			}
		}

		std::map<std::string, Body> bodies;
		std::vector<JointConnection> jointConnections;
		std::vector<GroundSupport> groundSupports;
	};

	template<int jointConnectionCount, int groundSupportCount>
	class System : public TSystem <jointConnectionCount, groundSupportCount, float> {
	public:
		void solve();
	};
}