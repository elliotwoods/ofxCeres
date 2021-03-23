#include "pch_ofxCeres.h"
#include "Builder.h"

template<typename T>
glm::tvec3<T> operator*(glm::tmat4x4<T> matrix, glm::tvec3<T> vec3)
{
	auto vec4 = matrix * glm::tvec4<T>(vec3.x, vec3.y, vec3.z, 1.0f);
	vec4 /= vec4.w;
	return glm::tvec3<T>(vec4.x, vec4.y, vec4.z);
}

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			namespace Builder {
				//----------
				void build(System & system
					, Chain & chain
					, map<string, function<shared_ptr<System::Body> ()>> factories) {

					std::string previousBodyName;
					shared_ptr<System::Body> previousBody;
					for (const auto & chainLink : chain) {
						// make the body
						auto newBody = factories[chainLink.bodyType]();
						system.bodies[chainLink.bodyName] = newBody;
						newBody->setGlobalOrientation(chainLink.orientation);

						// make joint connections
						if (!previousBodyName.empty()) {
							bool firstJointConnection = true;
							for (const auto & chainJoint : chainLink.jointConnections) {
								system.jointConnections.push_back({
									{ previousBodyName, chainJoint.jointNamePrevious }
									, { chainLink.bodyName, chainJoint.jointNameThis }
									});

								if (firstJointConnection) {
									// set the translation of the body if we have a joint
									auto positionPreviousJoint = previousBody->getGlobalTransformMatrix() * previousBody->joints[chainJoint.jointNamePrevious].position;
									auto positionThisJoint = newBody->getGlobalTransformMatrix() * newBody->joints[chainJoint.jointNameThis].position;
									newBody->setGlobalPosition(positionPreviousJoint - positionThisJoint);
								}

								firstJointConnection = false;
							}
						}

						// add ground supports
						for (const auto & jointName : chainLink.groundSupports) {
							system.groundSupports.push_back({ chainLink.bodyName, jointName });
						}

						previousBody = newBody;
						previousBodyName = chainLink.bodyName;
					}

					// initialise forces with random variables
					for (auto& connection : system.jointConnections) {
						connection.force.x = ofRandomf() * 0.01;
						connection.force.y = ofRandomf() * 0.01;
						connection.force.z = ofRandomf() * 0.01;
					}
					for (auto& connection : system.groundSupports) {
						connection.force.x = ofRandomf() * 0.01;
						connection.force.y = ofRandomf() * 0.01;
						connection.force.z = ofRandomf() * 0.01;
					}
				}
			}
		}
	}
}