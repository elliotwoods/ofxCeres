#pragma once

#include "Data/Serializable.h"
#include "ofxAssimpModelLoader.h"
#include "ofxCvGui.h"

class Mesh : public ofxCvGui::IInspectable, public Data::Serializable {
public:
	MAKE_ENUM(DrawStyle
		, (Mix, Fill, Wireframe)
		, ("Mix", "Fill", "Wireframe"));

	Mesh();
	string getTypeName() const override;

	void update();
	void drawWorld();

	void serialize(nlohmann::json&);
	void deserialize(const nlohmann::json&);
	void populateInspector(ofxCvGui::InspectArguments&);

	void loadMesh();
	bool isLoaded();
	glm::vec3 getMinWorld();
	glm::vec3 getMaxWorld();

	glm::mat4 getTransform() const;
	glm::vec3 getPointClosestTo(const glm::vec3&, float maxDistance);
protected:
	struct : ofParameterGroup {
		ofParameter<std::filesystem::path> filename{ "Filename", "" };
		ofParameter<float> scale{ "Scale", 1.0f, 0.0f, 1000.0f };

		struct : ofParameterGroup {
			ofParameter<float> x{ "X", 0, -180, 180 };
			ofParameter<float> y{ "Y", 0, -180, 180 };
			ofParameter<float> z{ "Z", 0, -180, 180 };
			PARAM_DECLARE("Rotation", x, y, z);
		} rotation;


		struct : ofParameterGroup {
			ofParameter<bool> enableMaterials{ "Enable materials", false };
			ofParameter<bool> enableTextures{ "Enable textures", true };

			struct : ofParameterGroup {
				ofParameter<bool> wireframe{ "Wireframe", false };
				ofParameter<bool> fill{ "Fill", true };
				PARAM_DECLARE("Cull back faces", wireframe, fill);
			} cullBackFaces;
			PARAM_DECLARE("Draw style", enableMaterials, enableTextures, cullBackFaces);
		} drawStyle;
		

		PARAM_DECLARE("Mesh", filename, scale, rotation, drawStyle);
	} parameters;

	ofParameter<DrawStyle> drawStyle{ "Draw style", DrawStyle::Mix };

	std::filesystem::path loadedPath;

	struct Model {
		ofMesh mesh;
		ofTexture texture;
		ofMaterial material;
	};

	vector<Model> models;
	glm::vec3 sceneMin;
	glm::vec3 sceneMax;
};
