#include "pch_ofApp.h"
#include "Pointe.h"

namespace DMX {
	//----------
	Pointe::Pointe()
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
			make_shared<Channel>("Pan", [this]() {
				return Fixture::get16bitMSB(this->parameters.pan, false);
				})
			, make_shared<Channel>("Pan fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.pan, false);
				})
			, make_shared<Channel>("Tilt", [this]() {
				return Fixture::get16bitMSB(this->parameters.tilt);
				})
			, make_shared<Channel>("Tilt fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.tilt);
				})
			, make_shared<Channel>("Pan/Tilt speed")
			, make_shared<Channel>("Power/Special functions")
			, make_shared<Channel>("Color wheel")
			, make_shared<Channel>("Color wheel fine")
			, make_shared<Channel>("Effect speed")
			, make_shared<Channel>("Static gobo wheel")
			, make_shared<Channel>("Rotating gobo wheel")
			, make_shared<Channel>("Rot. gobo indexing and rotation")
			, make_shared<Channel>("Rot. gobo indexing and rotation - fine")
			, make_shared<Channel>("Prism")
			, make_shared<Channel>("Prism rotation and indexing")
			, make_shared<Channel>("Frost")
			, make_shared<Channel>("Zoom", [this]() {
					return Fixture::get16bitMSB(this->customParameters.zoom);
					})
			, make_shared<Channel>("Zoom fine", [this]() {
					return Fixture::get16bitLSB(this->customParameters.zoom);
					})
			, make_shared<Channel>("Focus", [this]() {
					return Fixture::get16bitMSB(this->parameters.focus);
					})
			, make_shared<Channel>("Focus fine", [this]() {
					return Fixture::get16bitLSB(this->parameters.focus);
					})
			, make_shared<Channel>("Autofocus")
			, make_shared<Channel>("Shutter/strobe")
			, make_shared<Channel>("Dimmer intensity", [this]() {
					return Fixture::get16bitMSB(this->parameters.dimmer);
					})
			, make_shared<Channel>("Dimmer intensity fine", [this]() {
					return Fixture::get16bitLSB(this->parameters.dimmer);
					})
		};

		this->commands.push_back(Command{
			"Reset"
			, this->getChannelByName("Power/Special functions")
			, 205
			, 0
			, 5
			});
		this->commands.push_back(Command{
			"Lamp on"
			, this->getChannelByName("Power/Special functions")
			, 135
			, 0
			, 5
			});
		this->commands.push_back(Command{
			"Lamp off"
			, this->getChannelByName("Power/Special functions")
			, 235
			, 0
			, 5
			});
		this->commands.push_back(Command{
			"Eco mode"
			, this->getChannelByName("Power/Special functions")
			, 23
			, 0
			, 5
			});
		this->commands.push_back(Command{
			"Standard mode"
			, this->getChannelByName("Power/Special functions")
			, 27
			, 0
			, 5
			});
	}

	//----------
	string
		Pointe::getTypeName() const
	{
		return "DMX::Pointe";
	}

	//----------
	void
		Pointe::serialize(nlohmann::json& json)
	{

	}

	//----------
	void
		Pointe::deserialize(const nlohmann::json& json)
	{
	}

	//----------
	void
		Pointe::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;

		inspector->addParameterGroup(this->customParameters);

		inspector->addButton("Shutter open", [this]() {
			this->getChannelByName("Shutter/strobe")->setValue(255);
			});
		inspector->addButton("Shutter close", [this]() {
			this->getChannelByName("Shutter/strobe")->setValue(0);
			});
	}
}