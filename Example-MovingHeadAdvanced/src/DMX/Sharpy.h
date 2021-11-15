#pragma once
#include "MovingHead.h"

namespace DMX {
	class Sharpy : public MovingHead {
	public:
		Sharpy();
		string getTypeName() const override;

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);
	protected:
		struct {
			ofParameter<float> iris{ "Iris", 0, 0, 1 };
		} customParameters;
	};
}