#include "pch_ofApp.h"
#include "AbstractCalibrationPoint.h"

using namespace ofxCvGui;

namespace Data {
	//----------
	AbstractCalibrationPoint::AbstractCalibrationPoint() {
		this->selected.addListener(this, &AbstractCalibrationPoint::callbackSelectedChanged);

		this->onDeserialize += [this](const nlohmann::json & json) {
			{
				auto timestamp = (time_t)json["timestamp"];
				this->timestamp = chrono::system_clock::from_time_t(timestamp);
			}

			json >> this->selected;
			json >> this->color;
		};

		this->onSerialize += [this](nlohmann::json & json) {
			json["timestamp"] = chrono::system_clock::to_time_t(this->timestamp.get());
			json << this->selected;
			json << this->color;
		};

		{
			ofColor color = ofColor(200, 100, 100);
			color.setHueAngle(ofRandom(360.0));
			this->color.set(color);
		}

		this->onSelectionChanged += [this](bool &) {
			this->onChange.notifyListeners();
		};

		this->rebuildDateStrings();
	}

	//----------
	ofxCvGui::ElementPtr AbstractCalibrationPoint::getGuiElement() {
		auto element = make_shared<Element>();
		element->setHeight(55.0f);

		element->onDraw += [this](DrawArguments & args) {
			if (this->isSelected()) {
				ofPushStyle();
				{
					ofSetColor(50);
					ofDrawRectangle(args.localBounds);
				}
				ofPopStyle();
			}

			ofxAssets::font(ofxCvGui::getDefaultTypeface(), 22).drawString(timeString, 30, 28);
			ofxAssets::font(ofxCvGui::getDefaultTypeface(), 14).drawString(secondString, 95, 20);
			ofxAssets::font(ofxCvGui::getDefaultTypeface(), 10).drawString(dateString, 30, 45);
		};

		auto thirdHeight = element->getHeight() / 3.0f;

		{
			auto selectedButton = make_shared<Widgets::Toggle>(this->selected);
			selectedButton->setBounds(ofRectangle(2, 2, 25.0f, thirdHeight * 2.0f - 4));
			selectedButton->setCaption("");
			selectedButton->onDraw += [this](DrawArguments & args) {
				ofPushStyle();
				{
					ofSetColor(this->color);
					ofSetLineWidth(2.0f);
					if (this->isSelected()) {
						ofFill();
					}
					else {
						ofNoFill();
					}
					ofDrawCircle(args.localBounds.getCenter(), 5.0f);
				}
				ofPopStyle();
			};
			element->addChild(selectedButton);
		}

		{
			auto deleteButton = make_shared<Widgets::Button>("", [this]() {
				auto lockThis = this->shared_from_this();
				lockThis->onDeletePressed.notifyListeners();
			});

			deleteButton->setBounds(ofRectangle(2, thirdHeight * 2.0f + 3, 25.0f, thirdHeight - 3 - 2));
			deleteButton->onDraw += [this](DrawArguments & args) {
				ofPushStyle();
				{
					ofSetLineWidth(2.0f);
					ofPushMatrix();
					{
						ofTranslate(args.localBounds.getCenter());
						ofDrawLine(-3, -3, +3, +3);
						ofDrawLine(-3, +3, +3, -3);
					}
					ofPopMatrix();
				}
				ofPopStyle();
			};
			element->addChild(deleteButton);
		}

		auto dataDisplayElement = this->getDataDisplay();
		if (dataDisplayElement->getHeight() > element->getHeight()) {
			element->setHeight(dataDisplayElement->getHeight());
		}
		element->addChild(dataDisplayElement);

		auto elementWeak = weak_ptr<ofxCvGui::Element>(element);
		element->onBoundsChange += [dataDisplayElement, elementWeak](ofxCvGui::BoundsChangeArguments & args) {
			auto left = 120.0f;
			auto element = elementWeak.lock();
			dataDisplayElement->setPosition(glm::vec2(left, 0.0f));
			dataDisplayElement->setWidth(element->getWidth() - left);
			dataDisplayElement->setHeight(element->getHeight());
		};

		this->overlayMainDisplay(element);

		element->arrange();

		return element;
	}

	//----------
	bool AbstractCalibrationPoint::isSelected() const {
		return this->selected.get();
	}

	//----------
	void AbstractCalibrationPoint::setSelected(bool selected) {
		if (selected != this->selected) {
			this->selected = selected;
			this->onSelectionChanged(selected);
		}
	}

	//-----------
	void AbstractCalibrationPoint::rebuildDateStrings() {
		time_t time = chrono::system_clock::to_time_t(this->timestamp.get());

		{
			char buff[10];
			strftime(buff, 10, "%H:%M", localtime(&time));
			this->timeString = string(buff);
		}

		{
			char buff[5];
			strftime(buff, 5, ":%S", localtime(&time));
			this->secondString = string(buff);
		}

		{
			char buff[20];
			strftime(buff, 20, "%a %Y.%m.%d", localtime(&time));
			this->dateString = string(buff);
		}
	}

	//---------
	void AbstractCalibrationPoint::callbackSelectedChanged(bool & value) {
		this->onSelectionChanged.notifyListeners(value);
	}
}