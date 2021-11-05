#include "pch_ofApp.h"
#include "Channel.h"

namespace DMX {
	//----------
	Channel::Channel() :
		Channel("Value")
	{

	}

	//----------
	Channel::Channel(const string & name, DMX::Value defaultValue)
	{
		this->value.set(name, defaultValue, 0, 255);
	}

	//----------
	Channel::Channel(const string & name, function<DMX::Value()> generateValue)
		: Channel(name)
	{
		this->generateValue = generateValue;
	}

	//----------
	void
	Channel::update()
	{
		if (this->generateValue) {
			auto value = this->generateValue();
			this->value.set((float)value);
		}
	}

	//----------
	string
	Channel::getName() const
	{
		return this->value.getName();
	}

	//----------
	DMX::Value
	Channel::getValue() const
	{
		return this->value.get();
	}

	//----------
	void
	Channel::setValue(DMX::Value value)
	{
		this->value.set(value);
	}

	//----------
	ofxCvGui::ElementPtr
	Channel::getWidget()
	{
		if (this->generateValue) {
			return make_shared<ofxCvGui::Widgets::LiveValue<int>>(this->value.getName(), [this]() {
				return (int) this->value.get();
				});
		}
		else {
			auto slider = make_shared<ofxCvGui::Widgets::Slider>(this->value);
			slider->addIntValidator();
			return slider;
		}
	}

	//----------
	void
	Channel::serialize(nlohmann::json& json)
	{

	}

	//----------
	void
	Channel::deserialize(const nlohmann::json& json)
	{

	}

}