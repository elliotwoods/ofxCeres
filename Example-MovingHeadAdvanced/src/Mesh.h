#pragma once

#include "Data/Serializable.h"
#include "ofxAssimpModelLoader.h"
#include "ofxCvGui.h"

class Mesh : public ofxCvGui::IInspectable, public Data::Serializable {
public:
	Mesh();
	string getTypeName() const override;

	void drawWorld();

	void serialize(nlohmann::json&);
	void deserialize(const nlohmann::json&);
	void populateInspector(ofxCvGui::InspectArguments&);

protected:
	struct : ofParameterGroup {
		ofParameter<std::filesystem::path> filename{ "Filename", "" };
		ofParameter<float> scale{ "Scale", 0.01f };
	} parameters;

	ofxAssimpModelLoader model;
};
