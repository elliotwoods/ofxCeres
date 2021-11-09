#pragma once

#include "Marker.h"
#include "Markers.h"
#include "Mesh.h"
#include "GroupSolve.h"

#include "Data/CalibrationPointSet.h"
#include "DMX/MovingHead.h"
#include "DMX/EnttecUSBPro.h"

class Scene : public ofxCvGui::IInspectable {
public:
	Scene();
	void update();
	void drawWorld();
	void renderDMX();

	void serialize(nlohmann::json&);
	void deserialize(const nlohmann::json&);
	void populateInspector(ofxCvGui::InspectArguments&);
	void load(const string& path);
	void save(string& path);

	map<string, shared_ptr<DMX::MovingHead>> & getMovingHeads();
	void deleteMovingHead(const string&);

	shared_ptr<Markers> getMarkers();

	void mergeMarkers();
	void fitWorldGrid();
	void rotateScene();

	shared_ptr<ofxCvGui::Panels::WorldManaged> getPanel();

	static string getDefaultFilename();
protected:
	static shared_ptr<DMX::MovingHead> makeMovingHead(const string & typeName);

	map<string, shared_ptr<DMX::MovingHead>> movingHeads;
	shared_ptr<Markers> markers = make_shared<Markers>();
	shared_ptr<Mesh> mesh = make_shared<Mesh>();
	shared_ptr<GroupSolve> groupSolve = make_shared<GroupSolve>(*this);
	shared_ptr<DMX::EnttecUSBPro> enttecUSBPro = make_shared<DMX::EnttecUSBPro>();

	shared_ptr<ofxCvGui::Panels::WorldManaged> panel;

	struct {
		ofParameter<string> name{ "Name", "1" };
	} newMovingHead;

};