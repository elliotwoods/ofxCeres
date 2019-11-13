#include "pch_ofxCeres.h"
#include "System.h"
#include "ofMain.h"
#include "ofxCeres/Exception.h"

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			//----------
			ofxCeres::SolverSettings System::getDefaultSolverSettings() {
				auto solverSettings = ofxCeres::SolverSettings();
				auto & options = solverSettings.options;
				{
					options.linear_solver_type = ceres::DENSE_QR;
					options.minimizer_progress_to_stdout = solverSettings.printReport;
					options.function_tolerance = 1e-10;
					options.line_search_direction_type = ceres::LBFGS;
					options.minimizer_type = ceres::LINE_SEARCH;
					options.num_threads = 16;
				}
				return solverSettings;
			}

			//----------
			void System::draw() {
				for (auto & body : this->bodies) {
					if (!body.second->drawArgs.enabled) {
						continue;
					}

					ofDrawBitmapString(body.first, body.second->getPosition());
					body.second->draw();

					ofPushMatrix();
					{
						ofMultMatrix(body.second->getGlobalTransformMatrix());

						if (body.second->drawArgs.joints) {
							for (const auto & joint : body.second->joints) {
								auto position = joint.second.position;

								ofPushMatrix();
								{
									ofTranslate(position);
									ofDrawAxis(DrawProperties::X().anchorSize);
								}
								ofPopMatrix();


								auto force = joint.second.force;
								if (force.length() != 0) {
									ofPushStyle();
									{
#ifdef HAS_OFXCVGUI
										ofSetColor(DrawProperties::X().scalarToColor(glm::length(force)));
#endif
										auto arrowEnd = position + force * DrawProperties::X().getScalarToSceneScale();
										ofDrawArrow(position, arrowEnd, DrawProperties::X().arrowHeadSize);
									}
									ofPopStyle();
								}

								if (DrawProperties::X().jointLabels) {
									ofDrawBitmapString(joint.first, position);
								}
							}
						}

						if (body.second->drawArgs.loads) {
							for (const auto & load : body.second->loads) {
								auto position = load.second.position;

								ofPushMatrix();
								{
									ofTranslate(position);
									ofDrawAxis(DrawProperties::X().anchorSize);
								}
								ofPopMatrix();

								auto force = load.second.force;
								if (force.length() != 0) {
									ofPushStyle();
									{
#ifdef HAS_OFXCVGUI
										ofSetColor(DrawProperties::X().scalarToColor(glm::length(force)));
#endif
										auto arrowEnd = position + force * DrawProperties::X().getScalarToSceneScale();
										ofDrawArrow(position, arrowEnd, DrawProperties::X().arrowHeadSize);
									}
									ofPopStyle();
								}

								if (DrawProperties::X().loadLabels) {
									ofDrawBitmapString(load.first, position);
								}
							}
						}
					}
					ofPopMatrix();
				}
			}

			//-----------
			void System::throwIfBadJointConnection() const {
				auto checkJointExists = [this](const JointAddress & address) {
					auto findBody = this->bodies.find(address.bodyName);
					if (findBody == this->bodies.end()) {
						throw(ofxCeres::Exception("Joint address [" + address.toString() + "] does not exist within System"));
					}
					auto findJoint = findBody->second->joints.find(address.jointName);
					if (findJoint == findBody->second->joints.end()) {
						throw(ofxCeres::Exception("Joint address [" + address.toString() + "] does not exist within System"));
					}

				};
				for (const auto & jointConnection : this->jointConnections) {
					checkJointExists(jointConnection.A);
					checkJointExists(jointConnection.B);
				}
				for (const auto & groundSupport : this->groundSupports) {
					checkJointExists(groundSupport.A);
				}
			}

#ifdef HAS_OFXCVGUI
			//-----------
			void System::populateInspector(shared_ptr<ofxCvGui::Panels::Widgets> widgets) {
				widgets->addTitle("System");
				for (auto & bodyIt : this->bodies) {
					widgets->addTitle(bodyIt.first, ofxCvGui::Widgets::Title::H2);
					
					if (!bodyIt.second->joints.empty()) {
						widgets->addTitle("Joints", ofxCvGui::Widgets::Title::H3);
						{
							for (const auto & jointIt : bodyIt.second->joints) {
								auto & force = jointIt.second.force;
								widgets->addLiveValue<glm::vec3>(jointIt.first, [&]() {
									return force;
								});
							}
						}
					}

					if (!bodyIt.second->loads.empty()) {
						widgets->addTitle("Loads", ofxCvGui::Widgets::Title::H3);
						{
							for (const auto & loadIt : bodyIt.second->loads) {
								auto & force = loadIt.second.force;
								widgets->addLiveValue<glm::vec3>(loadIt.first, [&]() {
									return force;
								});
							}
						}
					}
				}
			}
#endif

		}
	}
}