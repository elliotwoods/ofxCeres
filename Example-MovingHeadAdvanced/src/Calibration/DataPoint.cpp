#include "pch_ofApp.h"
#include "DataPoint.h"

namespace Calibration {
	//----------
	DataPoint::DataPoint()
	{
		RULR_SERIALIZE_LISTENERS;
	}

	//----------
	string
	DataPoint::getTypeName() const
	{
		return "Calibration::DataPoint";
	}

	//----------
	void
	DataPoint::serialize(nlohmann::json& json)
	{
		json << this->panTiltSignal;
		json << this->marker;
	}

	//----------
	void
	DataPoint::deserialize(const nlohmann::json& json)
	{
		json >> this->panTiltSignal;
		json >> this->marker;
	}

	//----------
	int
	DataPoint::getQuadrant() const
	{
		const auto& panTilt = this->panTiltSignal.get();
		return (panTilt.x > 0.0f ? 1.0 : 0.0) + (panTilt.y > 0.0f ? 2.0 : 0.0);
	}

	//----------
	ofxCvGui::ElementPtr
	DataPoint::getDataDisplay()
	{
		auto element = ofxCvGui::makeElement();

		auto children = vector<ofxCvGui::ElementPtr>();
		
		// pan tilt widget
		{
			auto widget = make_shared<ofxCvGui::Widgets::EditableValue<glm::vec2>>(this->panTiltSignal);
			widget->onDraw.addListener([this](ofxCvGui::DrawArguments& args) {
				// Draw quadrant 
				{
					auto quadrantRect = args.localBounds;
					quadrantRect.width /= 2.0f;
					quadrantRect.height /= 2.0f;
					auto quadrant = this->getQuadrant();
					if (quadrant % 2) {
						quadrantRect.x += quadrantRect.width;
					}
					if (quadrant / 2) {
						quadrantRect.y += quadrantRect.height;
					}

					ofPushStyle();
					{
						ofSetColor(80);
						ofDrawRectangle(quadrantRect);

						// outline
						ofNoFill();
						ofDrawRectangle(args.localBounds);
					}
					ofPopStyle();
				}
			}, this, -1);
			widget->addToolTip("Background shading denotes quadrant");
			children.push_back(widget);
		}

		children.push_back(make_shared<ofxCvGui::Widgets::EditableValue<string>>(this->marker));

		// residual widget
		{
			auto residualWidget = make_shared<ofxCvGui::Widgets::LiveValue<float>>("Residual [degrees]", [this]() {
				return this->residual;
				});
			// draw bar underneath
			residualWidget->onDraw.addListener([this](ofxCvGui::DrawArguments& args) {
				ofPushStyle();
				{
					ofSetColor(100, 80, 80);
					auto bounds = args.localBounds;
					bounds.width *= this->normalisedResidual;
					ofDrawRectangle(bounds);
				}
				ofPopStyle();
				}, this, -1);

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
	void
	DataPoint::overlayMainDisplay(ofxCvGui::ElementPtr element)
	{
		float y = 60.0f;

		auto addButton = [element, &y](string caption, function<void()> action) {
			auto button = make_shared<ofxCvGui::Widgets::Button>(caption, action);

			button->setBounds({ 2, y, 110.0f, 30.0f });
			element->addChild(button);

			y += 35.0f;

			return button;
		};

		addButton("Go to stored pan/tilt", [this] {
			this->onGoValue.notifyListeners();
			})->setDrawGlyph(u8"\uf093");

		addButton("Go to calculated pan/tilt", [this] {
			try {
				this->onGoPrediction.notifyListeners();
			}
			CATCH_TO_ALERT;
			})->setDrawGlyph(u8"\uf1ec");

		addButton("Store the current pan/tilt", [this] {
			this->onTakeCurrent.notifyListeners();
			})->setDrawGlyph(u8"\uf019");

		// Expand height if needs be
		auto height = y;
		if (height > element->getHeight()) {
			element->setHeight(height);
		}
	}
}