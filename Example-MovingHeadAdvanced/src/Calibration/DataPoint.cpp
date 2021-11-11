#include "pch_ofApp.h"
#include "DataPoint.h"

namespace Calibration {
	//----------
	DataPoint::DataPoint() {
		RULR_SERIALIZE_LISTENERS;
	}

	//----------
	string DataPoint::getTypeName() const {
		return "Calibration::DataPoint";
	}

	//----------
	void DataPoint::serialize(nlohmann::json& json) {
		json << this->panTiltSignal;
		json << this->marker;
	}

	//----------
	void DataPoint::deserialize(const nlohmann::json& json) {
		json >> this->panTiltSignal;
		json >> this->marker;
	}

	//----------
	ofxCvGui::ElementPtr DataPoint::getDataDisplay() {
		auto element = ofxCvGui::makeElement();

		auto children = vector<ofxCvGui::ElementPtr>({
			make_shared<ofxCvGui::Widgets::EditableValue<glm::vec2>>(this->panTiltSignal)
			, make_shared<ofxCvGui::Widgets::EditableValue<string>>(this->marker)
			});

		{
			auto residualWidget = make_shared<ofxCvGui::Widgets::LiveValue<float>>("Residual [degrees]", [this]() {
				if (this->getResidualFunction) {
					try {
						return this->getResidualFunction(this);
					}
					catch (...) {
						// e.g. if this marker doesn't exist any more
						return 0.0f;
					}
				}
				else {
					return 0.0f;
				}
				});
			element->onUpdate += [residualWidget, this](ofxCvGui::UpdateArguments&) {
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

		element->onBoundsChange += [children](ofxCvGui::BoundsChangeArguments& args) {
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
	void DataPoint::overlayMainDisplay(ofxCvGui::ElementPtr element) {
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
			try {
				this->onGoPrediction.notifyListeners();
			}
			CATCH_TO_ALERT;
			});

		addButton("TAKE Current", [this] {
			this->onTakeCurrent.notifyListeners();
			});

		// Expand height if needs be
		auto height = y;
		if (height > element->getHeight()) {
			element->setHeight(height);
		}
	}
}