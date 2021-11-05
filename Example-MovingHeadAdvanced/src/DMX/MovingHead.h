#pragma once

#include "Fixture.h"

namespace DMX {
	class MovingHead : public Fixture {
	public:
		MovingHead();

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);

		struct : ofParameterGroup {
			ofParameter<float> pan{ "Pan", 0, -180, 180 };
			ofParameter<float> tilt{ "Tilt", 0, -90, 90 };
			ofParameter<float> dimmer{ "Dimmer", 0, 0, 1};
			ofParameter<float> focus{ "Focus", 0, 0, 1};
			ofParameter<float> iris{ "Iris", 0, 0, 1};
		} parameters;
	protected:

	};
}