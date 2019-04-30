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
		DataType value;
		json[parameter.getName()].get_to(value);
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
		string parameterValueString;
		json[parameter.getName()].get_to(parameterValueString);

		stringstream ss(parameterValueString);

		DataType value;
		ss >> value;
		parameter.set(value);
	}
}

//--serialization

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<string> & parameter) {
	serializeSimple(json, parameter);
	return json;
}

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<bool> & parameter) {
	serializeSimple(json, parameter);
	return json;
}

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<int> & parameter) {
	serializeSimple(json, parameter);
	return json;
}

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<float> & parameter) {
	serializeSimple(json, parameter);
	return json;
}

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<glm::vec2> & parameter) {
	serializeStream(json, parameter);
	return json;
}

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<glm::vec3> & parameter) {
	serializeStream(json, parameter);
	return json;
}

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<glm::vec4> & parameter) {
	serializeStream(json, parameter);
	return json;
}

nlohmann::json & operator<<(nlohmann::json & json, const ofParameter<ofColor> & parameter) {
	serializeStream(json, parameter);
	return json;
}

//--deserialization


const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<string> & parameter) {
	deserializeSimple(json, parameter);
	return json;
}

const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<bool> & parameter) {
	deserializeSimple(json, parameter);
	return json;
}

const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<int> & parameter) {
	deserializeSimple(json, parameter);
	return json;
}

const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<float> & parameter) {
	deserializeSimple(json, parameter);
	return json;
}

const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<glm::vec2> & parameter) {
	deserializeStream(json, parameter);
	return json;
}

const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<glm::vec3> & parameter) {
	deserializeStream(json, parameter);
	return json;
}

const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<glm::vec4> & parameter) {
	deserializeStream(json, parameter);
	return json;
}

const nlohmann::json & operator>>(const nlohmann::json & json, ofParameter<ofColor> & parameter) {
	deserializeStream(json, parameter);
	return json;
}