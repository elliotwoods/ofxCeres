#pragma once

#include "Types.h"
#include <vector>

#include "Data/Serializable.h"
#include "ofxCvGui.h"

namespace DMX {
	class EntecUSBPro : public ofxCvGui::IInspectable, public Data::Serializable {
	public:
		EntecUSBPro();
		string getTypeName() const override;

		bool open();
		void close();
		void send(const vector<DMX::Value>&);
		
		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);
	protected:
		ofParameter<string> port{ "Port", "COM1" };
		ofSerial serial;
	};
}