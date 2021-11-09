#include "pch_ofApp.h"
#include "MovingHead.h"

namespace DMX {
	//----------
	MovingHead::MovingHead()
	{
		this->parameters.setName("MovingHead");
		this->parameters.add(this->parameters.pan);
		this->parameters.add(this->parameters.tilt);
		this->parameters.add(this->parameters.dimmer);
		this->parameters.add(this->parameters.focus);
		this->parameters.add(this->parameters.iris);

		RULR_SERIALIZE_LISTENERS;

		this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
			this->populateInspector(args);
		};
	}

	//----------
	void
	MovingHead::serialize(nlohmann::json& json)
	{
		json << this->parameters.pan;
		json << this->parameters.tilt;
		json << this->parameters.dimmer;
		json << this->parameters.focus;
		json << this->parameters.iris;
	}

	//----------
	void
	MovingHead::deserialize(const nlohmann::json& json)
	{
		json >> this->parameters.pan;
		json >> this->parameters.tilt;
		json >> this->parameters.dimmer;
		json >> this->parameters.focus;
		json >> this->parameters.iris;
	}

	//----------
	void
	MovingHead::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
		inspector->addParameterGroup(this->parameters);
		inspector->addButton("Home", [this]() {
			this->parameters.pan.set(0);
			this->parameters.tilt.set(0);
			});
	}
}