#include "pch_ofApp.h"
#include "EnttecUSBPro.h"

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
	EnttecUSBPro::EnttecUSBPro()
	{
		RULR_SERIALIZE_LISTENERS;

		this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
			this->populateInspector(args);
		};

		this->parameters.add(this->parameters.port);
		this->parameters.add(this->parameters.open);
	}

	//----------
	string
	EnttecUSBPro::getTypeName() const
	{
		return "DMX::EnttecUSBPro";
	}

	//----------
	void
	EnttecUSBPro::update()
	{
		try {
			if (this->parameters.open.get() && !this->isOpen()) {
				if (!this->open()) {
					this->parameters.open.set(false);
					throw(Exception("Could not open serial port on " + this->parameters.port.get()));
				}
			}
			else if (!this->parameters.open.get() && this->isOpen()) {
				this->close();
			}
		}
		CATCH_TO_ALERT;
	}

	//----------
	bool
	EnttecUSBPro::open()
	{
		return this->serial.setup(this->parameters.port.get(), 57600);
	}

	//----------
	void
	EnttecUSBPro::close()
	{
		return this->serial.close();
	}

	//----------
	bool
	EnttecUSBPro::isOpen()
	{
		return this->serial.isInitialized();
	}

	//----------
	void
	EnttecUSBPro::send(const vector<DMX::Value>& values)
	{
		if (values.size() < 2) {
			return;
		}

		if (!this->serial.isInitialized()) {
			return;
		}

		// Gather DMX channels to send
		vector<unsigned char> contentToSend(&values[1], &values[values.size() - 1]);
		if (contentToSend.size() < 24) {
			contentToSend.resize(24);
		}
		ofBuffer buffer;

		auto writeByte = [&buffer](const unsigned char& byte) {
			buffer.append((const char *)&byte, 1);
		};
		auto writeBytes = [&buffer](const unsigned char * bytes, size_t length) {
			buffer.append((const char *) bytes, length);
		};

		writeByte((unsigned char)DMX_PRO_START_MSG);
		writeByte((unsigned char)DMX_PRO_SEND_PACKET);

		uint16_t length = 1 + contentToSend.size();
		{
			writeByte((unsigned char)(length & 0xff));
			writeByte((unsigned char)((length >> 8) & 0xff));
		}

		writeByte((unsigned char)DMX_START_CODE);
		writeBytes((unsigned char*)contentToSend.data(), contentToSend.size());
		writeByte((unsigned char)DMX_PRO_END_MSG);

		this->serial.writeBytes(buffer);
	}

	//----------
	void
	EnttecUSBPro::serialize(nlohmann::json& json)
	{
		Data::serialize(json, this->parameters);
	}

	//----------
	void
	EnttecUSBPro::deserialize(const nlohmann::json& json)
	{
		Data::deserialize(json, this->parameters);
	}

	//----------
	void
	EnttecUSBPro::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
		
		inspector->addParameterGroup(this->parameters);

		{
			inspector->addTitle("Select port:", ofxCvGui::Widgets::Title::H3);
			auto deviceList = this->serial.getDeviceList();
			for (auto& device : deviceList) {
				auto devicePath = device.getDevicePath();
				inspector->addButton(device.getDeviceName(), [this, devicePath]() {
					this->parameters.port.set(devicePath);
					if (this->isOpen()) {
						this->close();
					}
					});
			}
		}
	}
}