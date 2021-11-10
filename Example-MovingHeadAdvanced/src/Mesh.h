#pragma once

#include "Data/Serializable.h"
#include "ofxAssimpModelLoader.h"
#include "ofxCvGui.h"

class Mesh : public ofxCvGui::IInspectable, public Data::Serializable {
public:
	Mesh();
	string getTypeName() const override;

	void update();
	void drawWorld();

	void serialize(nlohmann::json&);
	void deserialize(const nlohmann::json&);
	void populateInspector(ofxCvGui::InspectArguments&);

	bool isLoaded();
	glm::vec3 getMin();
	glm::vec3 getMax();

	glm::mat4 getTransform() const;
	glm::vec3 getPointClosestTo(const glm::vec3&, float maxDistance);
protected:
	struct : ofParameterGroup {
		ofParameter<std::filesystem::path> filename{ "Filename", "" };
		ofParameter<float> scale{ "Scale", 1.0f };

		struct : ofParameterGroup {
			ofParameter<float> x{ "X", 0, -180, 180 };
			ofParameter<float> y{ "Y", 0, -180, 180 };
			ofParameter<float> z{ "Z", 0, -180, 180 };
			PARAM_DECLARE("Rotation", x, y, z);
		} rotation;

		ofParameter<bool> enableMaterials{ "Enable materials", false };

		PARAM_DECLARE("Mesh", filename, scale, rotation, enableMaterials);
	} parameters;

	std::filesystem::path loadedPath;
	ofxAssimpModelLoader model;
};
