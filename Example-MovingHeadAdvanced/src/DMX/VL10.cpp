#include "pch_ofApp.h"
#include "VL10.h"

namespace DMX {
	//----------
	VL10::VL10()
		: MovingHead(Configuration{
			-540/2, 540/2
			, -270/2, 270/2
			})
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;

		this->channels = {
			// 1
			make_shared<Channel>("Intensity MSB", [this]() {
				return Fixture::get16bitMSB(this->parameters.dimmer);
				})

			// 2
			, make_shared<Channel>("Intensity LSB", [this]() {
				return Fixture::get16bitLSB(this->parameters.dimmer);
				})

			// 2
			, make_shared<Channel>("Pan MSB", [this]() {
				return Fixture::get16bitMSB(this->parameters.pan, false);
				})
			// 4
			, make_shared<Channel>("Pan LSB", [this]() {
				return Fixture::get16bitLSB(this->parameters.pan, false);
				})

			// 5
			, make_shared<Channel>("Tilt MSB", [this]() {
				return Fixture::get16bitMSB(this->parameters.tilt);
				})
			// 6
			, make_shared<Channel>("Tilt LSB", [this]() {
				return Fixture::get16bitLSB(this->parameters.tilt);
				})

			// 7
			, make_shared<Channel>("Focus MSB", [this]() {
				return Fixture::get16bitMSB(this->parameters.focus);
			})
			// 8
			, make_shared<Channel>("Focus LSB", [this]() {
				return Fixture::get16bitLSB(this->parameters.focus);
			})

			// 8
			, make_shared<Channel>("Zoom MSB", [this]() {
				return Fixture::get16bitMSB(this->customParameters.zoom);
			})
			// 9
			, make_shared<Channel>("Zoom LSB", [this]() {
				return Fixture::get16bitLSB(this->customParameters.zoom);
			})

			// 11, 12, 13
			, make_shared<Channel>("Cyan")
			, make_shared<Channel>("Yellow")
			, make_shared<Channel>("Magenta")

			// 14, 15
			, make_shared<Channel>("Color Wheel")
			, make_shared<Channel>("Color Wheel Control")

			// 16, 17
			, make_shared<Channel>("Aperture Wheel", [this]() {
				return (DMX::Value) ofMap(this->customParameters.aperture.get(), 0, 1, 0, 25, true);
			})
			, make_shared<Channel>("Aperture Wheel Control")

			// 18, 19, 20, 21
			, make_shared<Channel>("Gobo Wheel 2")
			, make_shared<Channel>("Gobo Wheel 2 Rotation MSB")
			, make_shared<Channel>("Gobo Wheel 2 Rotation LSB")
			, make_shared<Channel>("Gobo Wheel 2 Control")

			// 22, 23, 24, 25
			, make_shared<Channel>("VLFX Wheel")
			, make_shared<Channel>("VLFX Wheel Rotation MSB")
			, make_shared<Channel>("VLFX Wheel Rotation LSB")
			, make_shared<Channel>("VLFX Wheel Control")

			// 26, 27, 28
			, make_shared<Channel>("Prism 1")
			, make_shared<Channel>("Prism 1 Rotation MSB")
			, make_shared<Channel>("Prism 1 Rotation LSB")

			// 29, 30, 31
			, make_shared<Channel>("Prism 2")
			, make_shared<Channel>("Prism 2 Rotation MSB")
			, make_shared<Channel>("Prism 2 Rotation LSB")

			// 32, 33
			, make_shared<Channel>("Frost 1")
			, make_shared<Channel>("Wash Mode")

			// 34, 35
			, make_shared<Channel>("Strobe Speed")
			, make_shared<Channel>("Strobe Control", [this]() {
				return this->parameters.shutterOpen.get()
					? 0
					: 8;
			})

			// 36, 37, 38, 39, 40
			, make_shared<Channel>("Focus Timing")
			, make_shared<Channel>("Optics Timing")
			, make_shared<Channel>("Color Timing")
			, make_shared<Channel>("Beam Timing")
			, make_shared<Channel>("Gobo Timing")

			// 41
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

		this->parameters.add(this->customParameters.zoom);
		this->parameters.add(this->customParameters.aperture);
	}

	//----------
	string
		VL10::getTypeName() const
	{
		return "DMX::VL10";
	}

	//----------
	void
		VL10::serialize(nlohmann::json& json)
	{

	}

	//----------
	void
		VL10::deserialize(const nlohmann::json& json)
	{
	}

	//----------
	void
		VL10::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
	}
}