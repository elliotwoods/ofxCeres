#include "pch_ofApp.h"
#include "MovingHeadDataPoint.h"

namespace Data {
	//----------
	MovingHeadDataPoint::MovingHeadDataPoint() {
		RULR_SERIALIZE_LISTENERS;
	}

	//----------
	string MovingHeadDataPoint::getTypeName() const {
		return "MovingHeadDataPoint";
	}

	//----------
	void MovingHeadDataPoint::drawWorld() const {
		ofPushStyle();
		{
			ofSetColor(this->color);
			ofDrawSphere(this->targetPoint.get(), 0.03);

			ofSetColor(255);
			ofDrawBitmapString(this->name, this->targetPoint.get());
		}
		ofPopStyle();
	}

	//----------
	void MovingHeadDataPoint::serialize(nlohmann::json & json) {
		json << this->name;
		json << this->dmxValues;
		json << this->targetPoint;
	}

	//----------
	void MovingHeadDataPoint::deserialize(const nlohmann::json & json) {
		json >> this->name;
		json >> this->dmxValues;
		json >> this->targetPoint;
	}

	//----------
	ofxCvGui::ElementPtr MovingHeadDataPoint::getDataDisplay() {
		auto element = ofxCvGui::makeElement();

		auto children = vector<ofxCvGui::ElementPtr>({
			make_shared<ofxCvGui::Widgets::EditableValue<string>>(this->name)
			, make_shared<ofxCvGui::Widgets::EditableValue<glm::vec4>>(this->dmxValues)
			, make_shared<ofxCvGui::Widgets::EditableValue<glm::vec3>>(this->targetPoint)
			, make_shared<ofxCvGui::Widgets::LiveValue<glm::vec2>>("Pan-Tilt angles", [this]() {
				return this->getPanTiltAngles();
			})
		});

		{
			auto residualWidget = make_shared<ofxCvGui::Widgets::LiveValue<float>>("Residual", [this]() {
				if (this->getResidualFunction) {
					return this->getResidualFunction(this);
				}
				else {
					return 0.0f;
				}
			});
			element->onUpdate += [residualWidget, this](ofxCvGui::UpdateArguments &) {
				if (this->getResidualFunction) {
					residualWidget->setEnabled(true);
				}
				else {
					residualWidget->setEnabled(false);
				}
			};

			children.push_back(residualWidget);
		}

		for (auto child : children) {
			element->addChild(child);
		}

		element->onBoundsChange += [children](ofxCvGui::BoundsChangeArguments & args) {
			auto bounds = args.localBounds;
			for (auto element : children) {
				bounds.height = element->getHeight();
				element->setBounds(bounds);
				bounds.y += bounds.height;
			}
		};

		element->setHeight(element->getChildren().size() * 40.0f);

		return element;
	}

	//----------
	glm::vec2 MovingHeadDataPoint::getPanTiltAngles() const {
		uint16_t panTotal = (uint16_t)this->dmxValues.get()[0] << 8;
		panTotal += (uint16_t)this->dmxValues.get()[1];

		uint16_t tiltTotal = (uint16_t)this->dmxValues.get()[2] << 8;
		tiltTotal += (uint16_t)this->dmxValues.get()[3];

		auto panRatio = (double)panTotal / (double)std::numeric_limits<uint16_t>::max();
		auto tiltRatio = (double)tiltTotal / (double)std::numeric_limits<uint16_t>::max();

		return glm::vec2{
			ofMap(panRatio, 0, 1.0f, -270, +270)
			, ofMap(tiltRatio, 0, 1.0f, -130, +130)
		};
	}
}