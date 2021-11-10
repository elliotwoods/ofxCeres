#include "pch_ofApp.h"
#include "PanTiltTrackpad.h"

using namespace ofxCvGui;

namespace Widgets {
	//----------
	PanTiltTrackpad::PanTiltTrackpad() {
		this->setHeight(300.0f);

		this->onDraw += [this](DrawArguments& args) {
			this->draw(args);
		};

		this->onMouse += [this](MouseArguments& args) {
			this->mouse(args);
		};

		this->onKeyboard += [this](KeyboardArguments& args) {
			this->keyboard(args);
		};
	}

	//----------
	PanTiltTrackpad::PanTiltTrackpad(ofParameter<glm::vec2> & parameter)
	: PanTiltTrackpad() {
		this->parameter = & parameter;
	}

	//----------
	PanTiltTrackpad::PanTiltTrackpad(ofParameter<float>& panParameter, ofParameter<float>& tiltParameter)
		: PanTiltTrackpad() {
		this->panParameter = &panParameter;
		this->tiltParameter = &tiltParameter;
	}

	//----------
	glm::vec2
	PanTiltTrackpad::toXY(const glm::vec2 & panTilt) {
		return glm::vec2(
			{
				ofMap(panTilt.x
					, this->getMin().x
					, this->getMax().x
					, this->getLocalBounds().getLeft()
					, this->getLocalBounds().getRight())
				, ofMap(panTilt.y
					, this->getMin().y
					, this->getMax().y
					, this->getLocalBounds().getBottom()
					, this->getLocalBounds().getTop())
			}
		);
	}

	//----------
	glm::vec2
	PanTiltTrackpad::get() const
	{
		if (this->parameter) {
			return this->parameter->get();
		}
		else {
			return glm::vec2(
				this->panParameter->get()
				, this->tiltParameter->get()
			);
		}
	}

	//----------
	glm::vec2
	PanTiltTrackpad::getMin() const
	{
		if (this->parameter) {
			return this->parameter->getMin();
		}
		else {
			return glm::vec2(
				this->panParameter->getMin()
				, this->tiltParameter->getMin()
			);
		}
	}

	//----------
	glm::vec2
	PanTiltTrackpad::getMax() const
	{
		if (this->parameter) {
			return this->parameter->getMax();
		}
		else {
			return glm::vec2(
				this->panParameter->getMax()
				, this->tiltParameter->getMax()
			);
		}
	}

	//----------
	void
	PanTiltTrackpad::set(const glm::vec2& value)
	{
		if (this->parameter) {
			this->parameter->set(value);
		}
		else {
			this->panParameter->set(value.x);
			this->tiltParameter->set(value.y);
		}
	}


	//----------
	void PanTiltTrackpad::draw(DrawArguments & args) {
		auto drawIsoPan = [&](const float & pan) {
			ofDrawLine(this->toXY({ pan, this->getMin().y }), this->toXY({ pan, this->getMax().y }));
			ofDrawBitmapString(ofToString(pan), toXY({pan, 0.0f }));
		};

		auto drawIsoTilt = [&](const float & tilt) {
			ofDrawLine(this->toXY({ this->getMin().x, tilt }), this->toXY({ this->getMax().x, tilt }));
			ofDrawBitmapString(ofToString(tilt), this->toXY({ 0.0f, tilt }));
		};

		// Draw the background of the selected position
		auto cursorForCurrent = this->toXY(this->get());
		ofPushStyle();
		{
			ofSetColor(250.0f);
			ofDrawCircle(cursorForCurrent, 10.0f);
		}
		ofPopStyle();

		ofxCvGui::Utils::ScissorManager::X().setScissorEnabled(false);

		ofPushStyle();
		{
			// Save some compute time since we're doing all the hardwork ourselves
			ofSetDrawBitmapMode(ofDrawBitmapMode::OF_BITMAPMODE_SIMPLE);

			ofSetColor(200);
			drawIsoPan(0);
			drawIsoTilt(0);

			drawIsoPan(this->getMin().x);
			drawIsoPan(this->getMax().x);
			drawIsoTilt(this->getMin().y);
			drawIsoTilt(this->getMax().y);

			ofSetColor(150);
			for (float pan = -90; pan > this->getMin().x; pan -= 90.0f) {
				drawIsoPan(pan);
			}
			for (float pan = +90; pan < this->getMax().x; pan += 90.0f) {
				drawIsoPan(pan);
			}
			for (float tilt = -90; tilt > this->getMin().y; tilt -= 90.0f) {
				drawIsoTilt(tilt);
			}
			for (float tilt = +90; tilt < this->getMax().y; tilt += 90.0f) {
				drawIsoTilt(tilt);
			}
		}
		ofPopStyle();

		ofxCvGui::Utils::ScissorManager::X().setScissorEnabled(true);

		// Draw the foreground of the selected position
		ofPushMatrix();
		{
			ofPushStyle();
			{
				ofSetColor(0);
				ofTranslate(cursorForCurrent);
				ofDrawLine(-5, -5, +5, +5);
				ofDrawLine(-5, +5, +5, -5);
			}
			ofPopStyle();
		}
		ofPopMatrix();
	}

	//----------
	void PanTiltTrackpad::mouse(MouseArguments & args) {
		args.takeMousePress(this);
		if (args.isDragging(this)) {
			float speed = args.button == 2 ? 0.01f : 0.2f;
			if (ofGetKeyPressed(OF_KEY_SHIFT)) {
				speed *= 10.0f;
			}

			auto delta = args.movement * speed * glm::vec2(1, -1);
			this->set(this->get() + delta);

			this->clampValue();
		}
	}

	//----------
	void PanTiltTrackpad::keyboard(KeyboardArguments & args) {
		if (args.action == KeyboardArguments::Pressed) {

			glm::vec2 delta{ 0.0, 0.0 };

			switch (args.key) {
			case OF_KEY_UP:
				delta.y = 1.0f;
				break;
			case OF_KEY_DOWN:
				delta.y = -1.0f;
				break;
			case OF_KEY_LEFT:
				delta.x = -1.0f;
				break;
			case OF_KEY_RIGHT:
				delta.x = 1.0f;
				break;
			default:
				break;
			}

			float speed = 0.02f;
			if (ofGetKeyPressed(OF_KEY_SHIFT)) {
				speed *= 10.0f;
			}

			delta = delta * speed;

			this->set(this->get() + delta);
			
			this->clampValue();
		}
	}

	//----------
	void PanTiltTrackpad::clampValue() {
		auto value = this->get();

		auto lessThan = glm::lessThan(value, this->getMin());
		auto moreThan = glm::greaterThan(value, this->getMin());
		auto outsideRange = glm::any(lessThan) || glm::any(moreThan);
		
		if (outsideRange) {
			this->set(glm::clamp(value, this->getMin(), this->getMax()));
		}
	}
}