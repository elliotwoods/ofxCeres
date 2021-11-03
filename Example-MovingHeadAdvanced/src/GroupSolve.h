#pragma once

#include "ofxCvGui.h"
#include "Data/Serializable.h"

class Scene;

class GroupSolve : public ofxCvGui::IInspectable, public Data::Serializable {
public:
	GroupSolve(Scene&);
	string getTypeName() const override;

	void serialize(nlohmann::json&);
	void deserialize(const nlohmann::json&);
	void populateInspector(ofxCvGui::InspectArguments&);

	void selectOnlyMultiUseMarkers();
	void solve();
protected:
	Scene& scene;
	ofxCeres::ParameterisedSolverSettings solverSettings;
};