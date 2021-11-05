#include "pch_ofApp.h"
#include "EntecUSBPro.h"

// from ofxDmx
#define DMX_PRO_HEADER_SIZE 4
#define DMX_PRO_START_MSG 0x7E
#define DMX_START_CODE 0
#define DMX_START_CODE_SIZE 1
#define DMX_PRO_SEND_PACKET 6 // "periodically send a DMX packet" mode
#define DMX_PRO_END_SIZE 1
#define DMX_PRO_END_MSG 0xE7

namespace DMX {
	//----------
	EntecUSBPro::EntecUSBPro()
	{
		RULR_SERIALIZE_LISTENERS;

		this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
			this->populateInspector(args);
		};
	}

	//----------
	string
	EntecUSBPro::getTypeName() const
	{
		return "DMX::EntecUSBPro";
	}

	//----------
	void
	EntecUSBPro::open()
	{
		this->serial.setup(this->port.get(), 57600);
	}

	//----------
	void
	EntecUSBPro::send(const vector<DMX::Value>& values)
	{
		if (values.size() < 2) {
			return;
		}

		if (!this->serial.isInitialized()) {
			ofLogError("DMX::EntecUSBPro") << "Cannot sent when not open";
		}

		this->serial.writeByte((unsigned char)DMX_PRO_START_MSG);
		this->serial.writeByte((unsigned char)DMX_PRO_SEND_PACKET);

		uint16_t length = (values.size() - 1) + 2;
		{
			this->serial.writeByte((unsigned char)(length & 0xff));
			this->serial.writeByte((unsigned char)((length >> 8) & 0xff));
		}

		this->serial.writeByte((unsigned char) DMX_START_CODE);
		this->serial.writeBytes((unsigned char*)(values.data() + 1), values.size() - 1);
		this->serial.writeByte((unsigned char)DMX_PRO_END_MSG);
	}
}