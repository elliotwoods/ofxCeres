#pragma once

#include "Types.h"
#include <vector>

#include "Data/Serializable.h"
#include "ofxCvGui.h"

namespace DMX {
	class EnttecUSBPro : public ofxCvGui::IInspectable, public Data::Serializable {
	public:
		EnttecUSBPro();
		string getTypeName() const override;

		void update();

		bool open();
		void close();
		bool isOpen();

		void send(const vector<DMX::Value>&);
		
		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);
	protected:
		struct : ofParameterGroup {
			ofParameter<string> port{ "Port", "COM1" };
			ofParameter<bool> open{ "Open", false };
		} parameters;

		ofSerial serial;
	};
}