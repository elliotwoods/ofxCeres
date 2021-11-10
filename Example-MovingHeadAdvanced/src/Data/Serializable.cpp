#include "pch_ofApp.h"
#include "Serializable.h"

using namespace std;

namespace Data {
	//----------
	string Serializable::getName() const {
		return this->getTypeName();
	}

	//----------
	void Serializable::notifySerialize(nlohmann::json & json) {
		this->onSerialize.notifyListeners(json);
	}

	//----------
	void Serializable::notifyDeserialize(const nlohmann::json & json) {
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
			this->notifySerialize(json);
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

				this->notifyDeserialize(json);
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
	if (json.contains(parameter.getName())) {
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
	if (json.contains(parameter.getName())) {
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


nlohmann::json&
operator<<(nlohmann::json& json, const ofParameter<std::filesystem::path>& parameter)
{
	json[parameter.getName()] = parameter.get().string();
	return json;
}
const nlohmann::json&
operator>>(const nlohmann::json& json, ofParameter<std::filesystem::path>& parameter)
{
	if (json.contains(parameter.getName())) {
		auto value = json[parameter.getName()].get<string>();
		parameter.set(filesystem::path(value));
	}
	return json;
}


#define TRY_SERIALIZE(Type) \
{ \
	auto typedParameter = dynamic_pointer_cast<ofParameter<Type>>(parameter); \
	if (typedParameter) { \
			jsonGroup << *typedParameter; \
			continue; \
	} \
}

#define TRY_DESERIALIZE(Type) \
{ \
	auto typedParameter = dynamic_pointer_cast<ofParameter<Type>>(parameter); \
	if (typedParameter) { \
			jsonGroup >> *typedParameter; \
			continue; \
	} \
}

namespace Data {
	void
	serialize(nlohmann::json& json, const ofParameterGroup& parameters) {
		auto& jsonGroup = parameters.getName().empty()
			? json
			: json[parameters.getName()];

		for (auto parameter : parameters) {
			TRY_SERIALIZE(string);
			TRY_SERIALIZE(bool);
			TRY_SERIALIZE(uint8_t);
			TRY_SERIALIZE(uint8_t);
			TRY_SERIALIZE(uint16_t);
			TRY_SERIALIZE(uint32_t);
			TRY_SERIALIZE(uint64_t);
			TRY_SERIALIZE(int8_t);
			TRY_SERIALIZE(int16_t);
			TRY_SERIALIZE(int32_t);
			TRY_SERIALIZE(int64_t);
			TRY_SERIALIZE(float);
			TRY_SERIALIZE(double);
			TRY_SERIALIZE(glm::vec2);
			TRY_SERIALIZE(glm::vec3);
			TRY_SERIALIZE(glm::vec4);
			TRY_SERIALIZE(ofColor);
			TRY_SERIALIZE(filesystem::path);
			{
				auto typedParameter = dynamic_pointer_cast<ofParameterGroup>(parameter);
				if (typedParameter) {
					Data::serialize(jsonGroup, *typedParameter);
				}
			}

			ofLogWarning("ofParamterGroup") << "Couldn't serialise " << parameter->getName();
		}
	}

	void
	deserialize(const nlohmann::json& json, ofParameterGroup& parameters) {
		if (!parameters.getName().empty() && !json.contains(parameters.getName())) {
			return;
		}

		const auto& jsonGroup = parameters.getName().empty()
			? json
			: json[parameters.getName()];

		for (auto parameter : parameters) {
			if (jsonGroup.contains(parameter->getName())) {
				TRY_DESERIALIZE(string);
				TRY_DESERIALIZE(bool);
				TRY_DESERIALIZE(uint8_t);
				TRY_DESERIALIZE(uint8_t);
				TRY_DESERIALIZE(uint16_t);
				TRY_DESERIALIZE(uint32_t);
				TRY_DESERIALIZE(uint64_t);
				TRY_DESERIALIZE(int8_t);
				TRY_DESERIALIZE(int16_t);
				TRY_DESERIALIZE(int32_t);
				TRY_DESERIALIZE(int64_t);
				TRY_DESERIALIZE(float);
				TRY_DESERIALIZE(double);
				TRY_DESERIALIZE(glm::vec2);
				TRY_DESERIALIZE(glm::vec3);
				TRY_DESERIALIZE(glm::vec4);
				TRY_DESERIALIZE(ofColor);
				TRY_DESERIALIZE(filesystem::path);
				{
					auto typedParameter = dynamic_pointer_cast<ofParameterGroup>(parameter);
					if (typedParameter) {
						Data::deserialize(jsonGroup, *typedParameter);
					}
				}

				ofLogWarning("ofParamterGroup") << "Couldn't deserialize " << parameter->getName();
			}
		}
	}
}