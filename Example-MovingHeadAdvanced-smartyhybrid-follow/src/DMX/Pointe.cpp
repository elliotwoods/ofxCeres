#include "pch_ofApp.h"
#include "Pointe.h"
//https://www.robe.cz/fileadmin/robe/downloads/dmx_charts/Robin_Pointe_DMX_charts.pdf

namespace DMX {
	//----------
	Pointe::Pointe()
		: MovingHead(Configuration{
			-270, 270
			, -135, 135
			})
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;
		
		MovingHead::parameters.add(this->customParameters.zoom);

		this->channels = {
			make_shared<Channel>("Pan", [this]() {
				return Fixture::get16bitMSB(this->parameters.pan, false);
				})
			, make_shared<Channel>("Pan Fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.pan, false);
				})
			, make_shared<Channel>("Tilt", [this]() {
				return Fixture::get16bitMSB(this->parameters.tilt);
				})
			, make_shared<Channel>("Tilt Fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.tilt);
				})
			, make_shared<Channel>("Pan/Tilt speed")
			, make_shared<Channel>("Power/Special functions")
			, make_shared<Channel>("Color wheel")
			, make_shared<Channel>("Color wheel Fine")
			, make_shared<Channel>("Effect speed")
			, make_shared<Channel>("Static gobo wheel")
			, make_shared<Channel>("Rotating gobo wheel")
			, make_shared<Channel>("Rot. gobo indexing and rotation")
			, make_shared<Channel>("Rot. gobo indexing and rotation - Fine")
			, make_shared<Channel>("Prism")
			, make_shared<Channel>("Prism rotation and indexing")
			, make_shared<Channel>("Frost")
			, make_shared<Channel>("Zoom", [this]() {
					return Fixture::get16bitMSB(this->customParameters.zoom);
					})
			, make_shared<Channel>("Zoom Fine", [this]() {
					return Fixture::get16bitLSB(this->customParameters.zoom);
					})
			, make_shared<Channel>("Focus", [this]() {
					return Fixture::get16bitMSB(this->parameters.focus);
					})
			, make_shared<Channel>("Focus Fine", [this]() {
					return Fixture::get16bitLSB(this->parameters.focus);
					})
			, make_shared<Channel>("Autofocus")
			, make_shared<Channel>("Shutter/strobe", [this]() {
				return this->parameters.shutter.get() ? 255 : 0;
					})
			, make_shared<Channel>("Dimmer intensity", [this]() {
					return Fixture::get16bitMSB(this->parameters.dimmer);
					})
			, make_shared<Channel>("Dimmer intensity Fine", [this]() {
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
	}
}
