#pragma once

#include "ofxCvGui.h"

namespace Widgets {
	class PanTiltTrackpad : public ofxCvGui::Element {
	public:
		PanTiltTrackpad(ofParameter<glm::vec2> & parameter);
		glm::vec2 toXY(const glm::vec2 &);
	protected:
		void draw(ofxCvGui::DrawArguments &);
		void mouse(ofxCvGui::MouseArguments &);
		void keyboard(ofxCvGui::KeyboardArguments &);

		void clampValue();

		ofParameter<glm::vec2> * parameter = nullptr;
	};
}