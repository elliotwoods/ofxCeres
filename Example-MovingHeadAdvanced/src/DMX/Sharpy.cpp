#include "pch_ofApp.h"
#include "Sharpy.h"

namespace DMX {
	//----------
	Sharpy::Sharpy()
	{
		RULR_SERIALIZE_LISTENERS;

		this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
			this->populateInspector(args);
		};

		this->channels = {
			make_shared<Channel>("Color Wheel")
			, make_shared<Channel>("Shutter")
			, make_shared<Channel>("Dimmer", [this]() {
				return ofMap(this->parameters.dimmer.get(), 0, 1, 0, 255, true);
				})
			, make_shared<Channel>("Gobo Wheel", [this]() {
					return ofMap(this->parameters.iris.get(), 0, 1, 11, 0, true);
					})
			, make_shared<Channel>("Prism Insertion")
			, make_shared<Channel>("Prism Rotation")
			, make_shared<Channel>("Effects Movement")
			, make_shared<Channel>("Frost")
			, make_shared<Channel>("Focus", [this]() {
				return ofMap(this->parameters.focus.get(), 0, 1, 0, 255, true);
				})
			, make_shared<Channel>("Pan", [this]() {
				auto panAll = (int)ofMap(this->parameters.pan.get(), this->parameters.pan.getMin(), this->parameters.pan.getMax(), 0, std::numeric_limits<uint16_t>::max());
				return (DMX::Value)(panAll >> 8);
				})
			, make_shared<Channel>("Pan Fine", [this]() {
				auto panAll = (int)ofMap(this->parameters.pan.get(), this->parameters.pan.getMin(), this->parameters.pan.getMax(), 0, std::numeric_limits<uint16_t>::max());
				return (DMX::Value)(panAll % 256);
				})
			, make_shared<Channel>("Tilt", [this]() {
				auto panAll = (int)ofMap(this->parameters.tilt.get(), this->parameters.tilt.getMin(), this->parameters.tilt.getMax(), 0, std::numeric_limits<uint16_t>::max());
				return (DMX::Value)(panAll >> 8);
				})
			, make_shared<Channel>("Tilt Fine", [this]() {
				auto panAll = (int)ofMap(this->parameters.tilt.get(), this->parameters.tilt.getMin(), this->parameters.tilt.getMax(), 0, std::numeric_limits<uint16_t>::max());
				return (DMX::Value)(panAll % 256);
				})
			, make_shared<Channel>("Function")
			, make_shared<Channel>("Reset")
			, make_shared<Channel>("Lamp Control")
		};

		this->commands.push_back(Command{
			"Reset"
			, this->getChannelByName("Reset")
			, 200
			, 0
			, 5
			});
		this->commands.push_back(Command{
			"Lamp on"
			, this->getChannelByName("Lamp Control")
			, 200
			, 0
			, 5
		});
		this->commands.push_back(Command{
			"Lamp off"
			, this->getChannelByName("Lamp Control")
			, 50
			, 0
			, 5
			});
	}

	//----------
	string
	Sharpy::getTypeName() const
	{
		return "DMX::Sharpy";
	}

	//----------
	void
		Sharpy::serialize(nlohmann::json& json)
	{

	}

	//----------
	void
		Sharpy::deserialize(const nlohmann::json& json)
	{
	}

	//----------
	void
		Sharpy::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;

		inspector->addTitle("Sharpy");

		inspector->addButton("Shutter open", [this]() {
			this->getChannelByName("Shutter")->setValue(104);
			});
		inspector->addButton("Shutter close", [this]() {
			this->getChannelByName("Shutter")->setValue(0);
			});
	}
}