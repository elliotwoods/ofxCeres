#pragma once

#include "ofxCvGui.h"
#include "Data/Serializable.h"

class Scene;

class GroupSolve : public ofxCvGui::IInspectable, public Data::Serializable {
public:
	GroupSolve(Scene&);
	string getTypeName() const override;

	void update();
	void drawWorld();

	void serialize(nlohmann::json&);
	void deserialize(const nlohmann::json&);
	void populateInspector(ofxCvGui::InspectArguments&);

	void prepareMarkers();
	void solve();
protected:
	Scene& scene;
	ofxCeres::ParameterisedSolverSettings solverSettings;

	struct Parameters : ofParameterGroup {
		ofParameter<bool> noDistortion{ "No distortion", false };
		ofParameter<bool> solveContinuously{ "Solve continuously", false };
		PARAM_DECLARE("GroupSolve", noDistortion, solveContinuously);
	} parameters;
};