#pragma once

#include "Channel.h"
#include "Data/Serializable.h"

namespace DMX {
	class Fixture : public ofxCvGui::IInspectable, public Data::Serializable{
	public:
		struct Command {
			std::string name;
			shared_ptr<Channel> channel;
			DMX::Value setValue;
			DMX::Value returnToValue = 0;

			float duration = 5;
			bool complete = false;
		};

		Fixture();
		ofParameter<DMX::ChannelIndex> channelIndex{ "Channel Index", 1, 1, 512 };
		vector<shared_ptr<Channel>> channels;

		void update();
		void getDMX(vector<DMX::Value>&);
		
		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);

		shared_ptr<Channel> getChannelByName(const string&);
	protected:
		vector<Command> commands;
		deque<Command> commandQueue;
		Command activeCommand;
	};
}
