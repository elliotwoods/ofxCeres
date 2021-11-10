#include "pch_ofApp.h"
#include "Fixture.h"
#include "Exception.h"

namespace DMX {
	//----------
	Fixture::Fixture()
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;

		// Set the active command to be completed by default
		this->activeCommand.complete = true;
	}

	//----------
	void
	Fixture::update()
	{
		for (const auto& channel : this->channels) {
			channel->update();
		}

		if (!activeCommand.complete) {
			this->activeCommand.duration -= ofGetLastFrameTime();
			if (this->activeCommand.duration < 0.0f) {
				this->activeCommand.complete = true;
			}
			if (this->activeCommand.complete) {
				this->activeCommand.channel->setValue(this->activeCommand.returnToValue);
			}
		}
		else {
			if (!this->commandQueue.empty()) {
				this->activeCommand = this->commandQueue.front();
				this->commandQueue.pop_front();
				this->activeCommand.channel->setValue(this->activeCommand.setValue);
			}
		}
	}

	//----------
	void
	Fixture::getDMX(vector<DMX::Value>& dmxUniverseValues)
	{
		auto channelIndex = this->channelIndex.get();
		auto endChannel = channelIndex + this->channels.size();
		if (endChannel > 513) {
			throw(Exception("DMX channels outside of range"));
		}

		if (endChannel > dmxUniverseValues.size()) {
			dmxUniverseValues.resize(endChannel);
		}

		for (const auto& channel : this->channels) {
			dmxUniverseValues[channelIndex] = channel->getValue();
			channelIndex++;
		}
	}

	//----------
	void
	Fixture::serialize(nlohmann::json& json)
	{
		json << this->channelIndex;

		{
			auto& jsonChannels = json["channels"];
			for (auto channel : this->channels) {
				channel->serialize(jsonChannels);
			}
		}
	}

	//----------
	void
	Fixture::deserialize(const nlohmann::json& json)
	{
		json >> this->channelIndex;

		if (json.contains("channels")) {
			const auto& jsonChannels = json["channels"];
			for (auto channel : this->channels) {
				channel->deserialize(jsonChannels);
			}
		}
	}

	//----------
	void
	Fixture::populateInspector(ofxCvGui::InspectArguments& args) {
		auto inspector = args.inspector;
		
		inspector->addTitle("Fixture");

		inspector->addEditableValue<DMX::ChannelIndex>(this->channelIndex);

		if (!this->commands.empty()) {
			inspector->addTitle("Commands", ofxCvGui::Widgets::Title::H3);

			for (auto command : this->commands) {
				inspector->addButton(command.name, [this, command]() {
					this->commandQueue.push_back(command); // push back a copy of the command
					});
			}

			inspector->addLiveValue<string>("Command queue", [this]() {
				stringstream result;
				if (!this->activeCommand.complete) {
					result << "[" << this->activeCommand.name << "] ";
				}
				for (int i = 0; i < this->commandQueue.size(); i++) {
					if (i != 0) {
						result << ", ";
					}
					result << this->commandQueue[i].name;
				}
				return result.str();
				});
		}

		inspector->addTitle("DMX Channels", ofxCvGui::Widgets::Title::H3);

		for (size_t i = 0; i < this->channels.size(); i++) {
			auto channel = this->channels[i];
			auto widget = channel->getWidget();
			widget->setCaption(ofToString(i + 1) + " : " + widget->getCaption());
			inspector->add(widget);
		}
	}

	//----------
	shared_ptr<Channel>
	Fixture::getChannelByName(const string& name)
	{
		for (const auto& channel : this->channels) {
			if (channel->getName() == name) {
				return channel;
			}
		}
		throw(Exception("Channel '" + name + "' not found."));
	}
}
