#include "pch_ofApp.h"
#include "System.h"

namespace Data {
	//----------
	void System::draw() {
		for (auto & body : this->bodies) {
			if (!body.second.drawArgs.enabled) {
				continue;
			}
			
			ofDrawBitmapString(body.first, body.second.getPosition());
			body.second.draw();

			ofPushMatrix();
			{
				ofMultMatrix(body.second.getGlobalTransformMatrix());

				if (body.second.drawArgs.joints) {
					for (const auto & joint : body.second.joints) {
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
								ofSetColor(DrawProperties::X().scalarToColor(glm::length(force)));
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

				if (body.second.drawArgs.loads) {
					for (const auto & load : body.second.loads) {
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
								ofSetColor(DrawProperties::X().scalarToColor(glm::length(force)));
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

	//----------
	void System::populateInspector(shared_ptr<ofxCvGui::Panels::Widgets> widgets) {
		widgets->addTitle("System");
		for (auto & body : this->bodies) {
			widgets->addTitle(body.first, ofxCvGui::Widgets::Title::H2);

			widgets->addTitle("Draw");
			{
				widgets->addToggle(body.second.drawArgs.enabled);
				widgets->addToggle(body.second.drawArgs.joints);
				widgets->addToggle(body.second.drawArgs.loads);
			}
		}
	}
}