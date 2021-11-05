#pragma once

#include "Types.h"

#include "ofParameter.h"
#include "ofxCvGui.h"

#include <string>
#include <functional>

namespace DMX {
	class Channel {
	public:
		Channel();
		Channel(const std::string& name, DMX::Value defaultValue = 0);
		Channel(const std::string& name, std::function<DMX::Value()> generateValue);

		void update();
		string getName() const;

		DMX::Value getValue() const;
		void setValue(DMX::Value);

		ofxCvGui::ElementPtr getWidget();

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
	protected:
		ofParameter<float> value{ "Channel", 0, 0, 255 }; // We use a float because then we can have a slider
		std::function<DMX::Value()> generateValue;
	};
}