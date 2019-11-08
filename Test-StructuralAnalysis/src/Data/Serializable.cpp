#include "pch_ofApp.h"
#include "Serializable.h"

using namespace std;

namespace Data {
	//----------
	string Serializable::getName() const {
		return this->getTypeName();
	}

	//----------
	void Serializable::serialize(nlohmann::json & json) {
		this->onSerialize.notifyListeners(json);
	}

	//----------
	void Serializable::deserialize(const nlohmann::json & json) {
		this->onDeserialize.notifyListeners(json);
	}

	//----------
	void Serializable::save(string filename) {
		if (filename == "") {
			auto result = ofSystemSaveDialog(this->getDefaultFilename() + ".json", "Save " + this->getTypeName());
			if (result.bSuccess) {
				filename = result.fileName;
			}
		}

		if (filename != "") {
			nlohmann::json json;
			this->serialize(json);
			ofFile output;
			output.open(filename, ofFile::WriteOnly, false);
			output << std::setw(4) << json << std::endl;
		}
	}

	//----------
	void Serializable::load(string filename) {
		if (filename == "") {
			auto result = ofSystemLoadDialog("Load " + this->getTypeName());
			if (result.bSuccess) {
				filename = result.fileName;
			}
		}

		if (filename != "") {
			try {
				ofFile input;
				input.open(ofToDataPath(filename, true), ofFile::ReadOnly, false);
				string jsonRaw = input.readToBuffer().getText();

				nlohmann::json json;
				input >> json;

				this->deserialize(json);
			}
			catch (std::exception e) {
				ofSystemAlertDialog(e.what());
			}
		}
	}

	//----------
	string Serializable::getDefaultFilename() const {
		auto name = this->getName();
		std::replace(name.begin(), name.end(), ':', '_');
		return name;
	}
}

// Check Rulr for how to do this in a much smarter way (just rushing for now)

template<typename DataType>
void serializeSimple(nlohmann::json & json, const ofParameter<DataType> & parameter) {
	json[parameter.getName()] = parameter.get();
}

template<typename DataType>
void deserializeSimple(const nlohmann::json & json, ofParameter<DataType> & parameter) {
	if (json.count(parameter.getName()) != 0) {
		auto value = json[parameter.getName()].template get<DataType>();
		parameter.set(value);
	}
}


template<typename DataType>
void serializeStream(nlohmann::json & json, const ofParameter<DataType> & parameter) {
	stringstream ss;
	ss << parameter.get();
	json[parameter.getName()] = ss.str();
}

template<typename DataType>
void deserializeStream(const nlohmann::json & json, ofParameter<DataType> & parameter) {
	if (json.count(parameter.getName()) != 0) {
        auto parameterValueString = json[parameter.getName()].template get<string>();
        
		stringstream ss(parameterValueString);

		DataType value;
		ss >> value;
		parameter.set(value);
	}
}

#define DEFINE_DS_SIMPLE(Type) \
nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<Type> & parameter) { \
	serializeSimple(json, parameter); \
	return json; \
} \
\
const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<Type> & parameter) { \
	deserializeSimple(json, parameter); \
	return json; \
}

#define DEFINE_DS_STREAM(Type) \
nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<Type> & parameter) { \
	serializeStream(json, parameter); \
	return json; \
} \
\
const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<Type> & parameter) { \
	deserializeStream(json, parameter); \
	return json; \
}
//--serialization

DEFINE_DS_SIMPLE(string)
DEFINE_DS_SIMPLE(bool)
DEFINE_DS_SIMPLE(uint8_t)
DEFINE_DS_SIMPLE(uint16_t)
DEFINE_DS_SIMPLE(uint32_t)
DEFINE_DS_SIMPLE(uint64_t)
DEFINE_DS_SIMPLE(int8_t)
DEFINE_DS_SIMPLE(int16_t)
DEFINE_DS_SIMPLE(int32_t)
DEFINE_DS_SIMPLE(int64_t)
DEFINE_DS_SIMPLE(float)
DEFINE_DS_SIMPLE(double)

DEFINE_DS_STREAM(glm::vec2)
DEFINE_DS_STREAM(glm::vec3)
DEFINE_DS_STREAM(glm::vec4)
DEFINE_DS_STREAM(ofColor)