#include "pch_ofxCeres.h"
#include "Builder.h"

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
				}
			}
		}
	}
}