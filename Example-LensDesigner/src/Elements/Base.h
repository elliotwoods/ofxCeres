#pragma once

#include "Data/Serializable.h"
#include "ofxCvGui.h"

namespace Elements {
	class Base : public Data::Serializable, public ofxCvGui::IInspectable {
	public:
		void init();
		string getName() const override;
		virtual string getGlyph() const = 0;

		glm::vec2 getPosition() const;
		void setPosition(const glm::vec2&);

		void drawObjectSpace() const;
		void drawWorldSpace() const;

		ofxLiquidEvent<void> onDrawObjectSpace;
	protected:
		void manageParameters(ofParameterGroup&);

		struct : ofParameterGroup {
			ofParameter<string> name{ "Name", "" };
			struct : ofParameterGroup {
				ofParameter<float> x{ "x", 0, -5, 5 };
				ofParameter<float> y{ "y", 0, -5, 5 };
				PARAM_DECLARE("Position", x, y);
			} position;
			PARAM_DECLARE("Element", name, position);
		} parameters;

		shared_ptr<ofxCvGui::Element> element;
	};
}