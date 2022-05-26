#include "pch_ofApp.h"
#include "SmartyHybrid.h"

namespace DMX {
	//----------
SmartyHybrid::SmartyHybrid()
	: MovingHead(Configuration{
		-270, 270
		, -135, 135
		})
	{
		RULR_SERIALIZE_LISTENERS;

		this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
			this->populateInspector(args);
		};

		this->channels = {
             make_shared<Channel>("Pan", [this]() {
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
            , make_shared<Channel>("Cyan")
            , make_shared<Channel>("Cyan Fine")
            , make_shared<Channel>("Magenta")
            , make_shared<Channel>("Magenta Fine")
            , make_shared<Channel>("Yellow")
            , make_shared<Channel>("Yellow Fine")
            , make_shared<Channel>("Color Wheel")
            , make_shared<Channel>("Color Wheel Fine")
            , make_shared<Channel>("Rotating Gobo Continous")
            , make_shared<Channel>("Rotating Gobo Wheel")
            , make_shared<Channel>("Rotating Gobo Wheel Fine")
            , make_shared<Channel>("Static Gobo Wheel")
            , make_shared<Channel>("Static Gobo Wheel Fine")
            , make_shared<Channel>("Rotating Prisms Macro")
            , make_shared<Channel>("Rotating Prisms Index")
            , make_shared<Channel>("Rotating Prisms Index Fine")
            , make_shared<Channel>("Focus", [this]() {
                    return Fixture::get16bitMSB(this->parameters.focus);
                    })
            , make_shared<Channel>("Focus Fine", [this]() {
                    return Fixture::get16bitLSB(this->parameters.focus);
                    })
            , make_shared<Channel>("Zoom", [this]() {
                    return Fixture::get16bitMSB(this->customParameters.zoom);
                    })
            , make_shared<Channel>("Zoom Fine", [this]() {
                    return Fixture::get16bitLSB(this->customParameters.zoom);
                    })
            , make_shared<Channel>("Autofocus")
            , make_shared<Channel>("Autofocus Fine")
            , make_shared<Channel>("Shutter Strobe", [this]() {
                return this->parameters.shutter.get() ? 255 : 0;
                    })
			, make_shared<Channel>("Dimmer", [this]() {
				return Fixture::get16bitMSB(this->parameters.dimmer);
				})
            , make_shared<Channel>("Dimmer Fine", [this]() {
                return Fixture::get16bitLSB(this->parameters.dimmer);
                })
            , make_shared<Channel>("Frost")
			, make_shared<Channel>("Gobo Color Speed")
            , make_shared<Channel>("Color Macros")
            , make_shared<Channel>("Pan/Tilt speed")
            , make_shared<Channel>("Lamp Internal Programs")
        };
        
        this->commands.push_back(Command{
            "Lamp on"
            , this->getChannelByName("Lamp Internal Programs")
            , 40
            , 0
            , 10 //5 seconds was not enough for some reason
        });
        this->commands.push_back(Command{
            "Lamp off"
            , this->getChannelByName("Lamp Internal Programs")
            , 50
            , 0
            , 10
        });
        this->commands.push_back(Command{
            "Reset All Motors"
            , this->getChannelByName("Lamp Internal Programs")
            , 80
            , 0
            , 10
        });
        this->commands.push_back(Command{
            "Reset Dimmer"
            , this->getChannelByName("Lamp Internal Programs")
            , 94
            , 0
            , 10
        });
     
        
        this->parameters.add(this->customParameters.zoom);
    }

	//----------
	string
SmartyHybrid::getTypeName() const
	{
		return "DMX::SmartyHybrid";
	}

	//----------
	void
SmartyHybrid::serialize(nlohmann::json& json)
	{

	}

	//----------
	void
SmartyHybrid::deserialize(const nlohmann::json& json)
	{
	}

	//----------
	void
SmartyHybrid::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
	}
}
