#pragma once
#include "MovingHead.h"

namespace DMX {
	class Pointe : public MovingHead {
	public:
		Pointe();
		string getTypeName() const override;

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);
	protected:
		struct : ofParameterGroup {
			ofParameter<float> zoom{ "Zoom", 0, 0, 1 };
			PARAM_DECLARE("Pointe", zoom);
		} customParameters;
	};
}