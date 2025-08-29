#pragma once
#include "MovingHead.h"

namespace DMX {
	class VL6000 : public MovingHead {
	public:
		VL6000();
		string getTypeName() const override;

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);
	protected:
	};
}