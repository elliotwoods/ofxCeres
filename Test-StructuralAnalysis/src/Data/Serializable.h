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

namespace Data {
	class Serializable {
	public:
		virtual std::string getTypeName() const = 0;
		virtual std::string getName() const;

		ofxLiquidEvent<nlohmann::json> onSerialize;
		ofxLiquidEvent<const nlohmann::json> onDeserialize;
		void serialize(nlohmann::json &);
		void deserialize(const nlohmann::json &);

		void save(std::string filename = "");
		void load(std::string filename = "");
		std::string getDefaultFilename() const;
	};
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
nlohmann::json & operator<<(nlohmann::json &, const ofParameter<ofColor> &);

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
const nlohmann::json & operator>>(const nlohmann::json &, ofParameter<ofColor> &);
