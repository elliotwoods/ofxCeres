#pragma once
#include "MovingHead.h"

namespace DMX {
	class VL10 : public MovingHead {
	public:
		VL10();
		string getTypeName() const override;

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);
	protected:
		struct {
			ofParameter<float> zoom{ "Zoom", 0, 0, 1 };
			ofParameter<float> aperture{ "Aperture", 0, 0, 1 };
		} customParameters;
	};
}