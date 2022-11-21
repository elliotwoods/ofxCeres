#pragma once

#include "Channel.h"
#include "Data/Serializable.h"
#include "OSC/Router.h"

namespace DMX {
	class Fixture : public ofxCvGui::IInspectable, public Data::Serializable, public OSC::Router {
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

		virtual void update();
		void getDMX(vector<DMX::Value>&);

		virtual void drawWorld() = 0;
		
		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);

		shared_ptr<Channel> getChannelByName(const string&);
	protected:
		static uint16_t getAll(const ofParameter<float>&, bool invert = false);
		static DMX::Value get16bitMSB(const ofParameter<float>&, bool invert=false);
		static DMX::Value get16bitLSB(const ofParameter<float>&, bool invert=false); // 'Fine'
		static DMX::Value get8bit(const ofParameter<float>&, bool invert = false);

		vector<Command> commands;
		deque<Command> commandQueue;
		Command activeCommand;
	};
}
