#include "pch_ofApp.h"
#include "Sharpy.h"

namespace DMX {
	//----------
	Sharpy::Sharpy()
	: MovingHead(Configuration{
		-270, 270
		, -125, 125
		})
	{
		RULR_SERIALIZE_LISTENERS;

		this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
			this->populateInspector(args);
		};

		this->channels = {
			make_shared<Channel>("Color Wheel")
			, make_shared<Channel>("Shutter", [this]() {
				return this->parameters.shutterOpen.get() ? 255 : 0;
				})
			, make_shared<Channel>("Dimmer", [this]() {
				return Fixture::get8bit(this->parameters.dimmer);
				})
			, make_shared<Channel>("Gobo Wheel", [this]() {
				return ofMap(this->customParameters.iris.get(), 0, 1, 11, 0, true);
				})
			, make_shared<Channel>("Prism Insertion")
			, make_shared<Channel>("Prism Rotation")
			, make_shared<Channel>("Effects Movement")
			, make_shared<Channel>("Frost")
			, make_shared<Channel>("Focus", [this]() {
				return Fixture::get8bit(this->parameters.focus);
				})
			, make_shared<Channel>("Pan", [this]() {
				return Fixture::get16bitMSB(this->parameters.pan, true);
				})
			, make_shared<Channel>("Pan Fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.pan, true);
				})
			, make_shared<Channel>("Tilt", [this]() {
				return Fixture::get16bitMSB(this->parameters.tilt, false);
				})
			, make_shared<Channel>("Tilt Fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.tilt, false);
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

		this->parameters.add(this->customParameters.iris);
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
	}
}