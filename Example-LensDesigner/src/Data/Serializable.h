#pragma once

#include "ofMain.h"
#include "ofxLiquidEvent.h"

#define RULR_SERIALIZE_LISTENERS \
	this->onSerialize += [this](nlohmann::json & json) { \
		this->serialize(json); \
	}; \
	this->onDeserialize += [this](const nlohmann::json & json) { \
		this->deserialize(json); \
	}

#define RULR_INSPECTOR_LISTENER \
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) { \
		this->populateInspector(args); \
	};

namespace Data {
	class Serializable {
	public:
		virtual std::string getTypeName() const = 0;
		virtual std::string getName() const;

		ofxLiquidEvent<nlohmann::json> onSerialize;
		ofxLiquidEvent<const nlohmann::json> onDeserialize;
		void notifySerialize(nlohmann::json &);
		void notifyDeserialize(const nlohmann::json &);

		void save(std::string filename = "");
		void load(std::string filename = "");
		std::string getDefaultFilename() const;

		
	};

	void serialize(nlohmann::json&, const ofParameterGroup&);
	void deserialize(const nlohmann::json&, ofParameterGroup&);

	template<typename EnumType>
	void serializeEnum(nlohmann::json& json, const ofParameter<EnumType>& parameter)
	{
		json[parameter.getName()] = parameter.get().toString();
	}

	template<typename EnumType>
	void deserializeEnum(const nlohmann::json& json, ofParameter<EnumType>& parameter)
	{
		if (json.contains(parameter.getName())) {
			auto valueString = json[parameter.getName()].get<string>();
			auto value = parameter.get();
			value.fromString(valueString);
			parameter.set(value);
		}
	}

}

nlohmann::json & operator<<(nlohmann::json &, const ofParameter<string> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<bool> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<uint8_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<uint16_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<uint32_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<uint64_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<int8_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<int16_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<int32_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<int64_t> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<float> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<double> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<glm::vec2> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<glm::vec3> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<glm::vec4> &);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<ofColor>&);
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<std::filesystem::path>&);

const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<string> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<bool> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<uint8_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<uint16_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<uint32_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<uint64_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<int8_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<int16_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<int32_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<int64_t> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<float> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<double> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<glm::vec2> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<glm::vec3> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<glm::vec4> &);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<ofColor>&);
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<std::filesystem::path> &);

