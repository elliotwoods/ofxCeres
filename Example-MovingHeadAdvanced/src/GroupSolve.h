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

	void prepareMarkers();
	void solve();
protected:
	Scene& scene;
	ofxCeres::ParameterisedSolverSettings solverSettings;

	struct Parameters : ofParameterGroup {
		ofParameter<bool> noDistortion{ "No distortion", false };
		Parameters(){
			this->setName("GroupSolve");
			this->add(this->noDistortion);
		}
	} parameters;
};