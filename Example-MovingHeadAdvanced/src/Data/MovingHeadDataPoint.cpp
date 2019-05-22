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
		json << this->panTiltSignal;
		json << this->targetPoint;
	}

	//----------
	void MovingHeadDataPoint::deserialize(const nlohmann::json & json) {
		json >> this->name;
		json >> this->panTiltSignal;
		json >> this->targetPoint;
	}

	//----------
	ofxCvGui::ElementPtr MovingHeadDataPoint::getDataDisplay() {
		auto element = ofxCvGui::makeElement();

		auto children = vector<ofxCvGui::ElementPtr>({
			make_shared<ofxCvGui::Widgets::EditableValue<string>>(this->name)
			, make_shared<ofxCvGui::Widgets::EditableValue<glm::vec2>>(this->panTiltSignal)
			, make_shared<ofxCvGui::Widgets::EditableValue<glm::vec3>>(this->targetPoint)
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

		element->setHeight(element->getChildren().size() * 40.0f + 5.0f);

		return element;
	}

	//----------
	void MovingHeadDataPoint::overlayMainDisplay(ofxCvGui::ElementPtr element) {
		float y = 60.0f;

		auto addButton = [element, &y](string caption, function<void()> action) {
			auto button = make_shared<ofxCvGui::Widgets::Button>(caption, action);

			button->setBounds({ 2, y, 110.0f, 30.0f });
			element->addChild(button);

			y += 35.0f;
		};

		addButton("GO Value", [this] {
			this->onGoValue.notifyListeners();
		});

		addButton("GO Prediction", [this] {
			this->onGoPrediction.notifyListeners();
		});

		addButton("TAKE Current", [this] {
			this->onTakeCurrent.notifyListeners();
		});

		element->onDraw += [this](ofxCvGui::DrawArguments & args) {
			if (this->isFocused) {
				if (this->isFocused()) {
					// Draw an outline
					ofPushStyle();
					{
						ofNoFill();
						ofDrawRectangle(args.localBounds);
					}
					ofPopStyle();
				}
			}
		};

		element->onMouse += [this](ofxCvGui::MouseArguments & args) {
			if (args.isLocal() && args.action == ofxCvGui::MouseArguments::Pressed) {
				this->onRequestFocus.notifyListeners();
			}
		};
	}
}