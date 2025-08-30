#include "pch_ofApp.h"
#include "VL6000.h"

namespace DMX {
	//----------
	VL6000::VL6000()
		: MovingHead(Configuration{
			-270, 270
			, -121, 121
			})
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;

		this->channels = {
			// 1
			make_shared<Channel>("Dimmer intensity", [this]() {
				return Fixture::get8bit(this->parameters.dimmer);
				})

			// 2
			, make_shared<Channel>("Pan", [this]() {
				return Fixture::get16bitMSB(this->parameters.pan, false);
				})
			// 3
			, make_shared<Channel>("Pan fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.pan, false);
				})

			// 4
			, make_shared<Channel>("Tilt", [this]() {
				return Fixture::get16bitMSB(this->parameters.tilt);
				})
			// 5
			, make_shared<Channel>("Tilt fine", [this]() {
				return Fixture::get16bitLSB(this->parameters.tilt);
				})

			// 6
			, make_shared<Channel>("Edge", [this]() {
				return this->parameters.focus.get();
			})

			// 7
			, make_shared<Channel>("Color Wheel Presets")

			// 8
			, make_shared<Channel>("Color Wheel Transition")

			// 9
			, make_shared<Channel>("Color Wheel 1")

			// 10
			, make_shared<Channel>("Color Wheel 1 Control")

			// 11
			, make_shared<Channel>("Color Wheel 2")

			// 12
			, make_shared<Channel>("Color Wheel 2 Control")

			// 13
			, make_shared<Channel>("Color Wheel 3")

			// 14
			, make_shared<Channel>("Color Wheel 3 Control")

			// 15
			, make_shared<Channel>("Gobo Wheel 1")

			// 16
			, make_shared<Channel>("Gobo Wheel 1 Rotate MSB")

			// 17
			, make_shared<Channel>("Gobo Wheel 1 Rotate LSB")

			// 18
			, make_shared<Channel>("Gobo Wheel 1 Control")

			// 19
			, make_shared<Channel>("Iris")

			// 20
			, make_shared<Channel>("Frost")

			// 21
			, make_shared<Channel>("Strobe Speed")

			// 22
			, make_shared<Channel>("Strobe Control", [this]() {
					return this->parameters.shutterOpen.get()
						? 0
						: 8;
			})

			// 23
			, make_shared<Channel>("Luminaire Control")
		};

		this->commands.push_back(Command{
			"Lamp on"
			, this->getChannelByName("Luminaire Control")
			, 13
			, 0
			, 5
			});
		this->commands.push_back(Command{
			"Lamp off"
			, this->getChannelByName("Luminaire Control")
			, 18
			, 0
			, 5
			});
		this->commands.push_back(Command{
			"Recalibrate"
			, this->getChannelByName("Luminaire Control")
			, 8
			, 0
			, 5
			});
	}

	//----------
	string
		VL6000::getTypeName() const
	{
		return "DMX::VL6000";
	}

	//----------
	void
		VL6000::serialize(nlohmann::json& json)
	{

	}

	//----------
	void
		VL6000::deserialize(const nlohmann::json& json)
	{
	}

	//----------
	void
		VL6000::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
	}
}