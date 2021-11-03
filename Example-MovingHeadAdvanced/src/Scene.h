#pragma once

#include "MovingHead.h"
#include "Marker.h"
#include "Mesh.h"
#include "GroupSolve.h"

#include "Data/CalibrationPointSet.h"

class Scene : public ofxCvGui::IInspectable {
public:
	Scene();
	void update();
	void drawWorld();
	void renderDMX(vector<uint8_t>&);

	void populateInspector(ofxCvGui::InspectArguments&);
	void load();
	void save();

	map<string, shared_ptr<MovingHead>> & getMovingHeads();
	shared_ptr<Markers> getMarkers();

	shared_ptr<MovingHead> addMovingHead(const string & name);
	void addMovingHead(const string& name, shared_ptr<MovingHead>);
	void deleteMovingHead(const string& name);
	void importMovingHead();

	void mergeMarkers();
	void fitWorldGrid();
	void rotateScene();

	shared_ptr<ofxCvGui::Panels::WorldManaged> getPanel();
protected:
	map<string, shared_ptr<MovingHead>> movingHeads;
	shared_ptr<Markers> markers = make_shared<Markers>();
	shared_ptr<Mesh> mesh = make_shared<Mesh>();
	shared_ptr<GroupSolve> groupSolve = make_shared<GroupSolve>(*this);

	string selection = "";
	ofParameter<bool> drawOtherFixtures{ "Draw other fixtures", true };

	shared_ptr<ofxCvGui::Panels::WorldManaged> panel;
};