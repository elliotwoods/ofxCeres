#pragma once

#include "ofxCvGui.h"

namespace Widgets {
	class PanTiltTrackpad : public ofxCvGui::Element {
	private:
		PanTiltTrackpad();
	public:
		PanTiltTrackpad(ofParameter<glm::vec2>& parameter);
		PanTiltTrackpad(ofParameter<float>& panParameter, ofParameter<float>& tiltParameter);
		glm::vec2 toXY(const glm::vec2 &);

		glm::vec2 get() const;
		glm::vec2 getMin() const;
		glm::vec2 getMax() const;
		void set(const glm::vec2&);
	protected:
		void draw(ofxCvGui::DrawArguments &);
		void mouse(ofxCvGui::MouseArguments &);
		void keyboard(ofxCvGui::KeyboardArguments &);

		void clampValue();

		ofParameter<glm::vec2>* parameter = nullptr;

		ofParameter<float>* panParameter = nullptr;
		ofParameter<float>* tiltParameter = nullptr;
	};
}