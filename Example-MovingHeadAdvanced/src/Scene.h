#pragma once

#include "Marker.h"
#include "Markers.h"
#include "Mesh.h"
#include "GroupSolve.h"
#include "GroupControl.h"

#include "Data/CalibrationPointSet.h"
#include "DMX/MovingHead.h"
#include "DMX/EnttecUSBPro.h"

#include "OSC/Router.h"
#include "ofxOscReceiver.h"

class Scene : public ofxCvGui::IInspectable, public OSC::Router {
private:
	Scene();
public:
	static shared_ptr<Scene> X(); // Scene is a global singleton

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
	void renameMovingHead(const string&);

	shared_ptr<Markers> getMarkers();

	void mergeMarkers();
	void fitWorldGrid();
	void rotateScene();

	shared_ptr<ofxCvGui::Panels::WorldManaged> getPanel();

	static string getDefaultFilename();
protected:
	static shared_ptr<DMX::MovingHead> makeMovingHead(const string & typeName);

	map<string, shared_ptr<DMX::MovingHead>> movingHeads;
	shared_ptr<Mesh> mesh = make_shared<Mesh>();
	shared_ptr<GroupSolve> groupSolve = make_shared<GroupSolve>(*this);
	shared_ptr<DMX::EnttecUSBPro> enttecUSBPro = make_shared<DMX::EnttecUSBPro>();
	shared_ptr<ofxCvGui::Panels::WorldManaged> panel = ofxCvGui::Panels::makeWorldManaged();
	shared_ptr<Markers> markers = make_shared<Markers>(mesh, panel);
	shared_ptr<GroupControl> groupControl = make_shared<GroupControl>();

	shared_ptr<ofxOscReceiver> oscReceiver;

	struct {
		ofParameter<string> name{ "Name", "1" };
		ofParameter<DMX::ChannelIndex> dmxChannelIndex{ "Dmx channel index", 1 };
	} newMovingHead;

	struct : ofParameterGroup {
		ofParameter<bool> enabled{ "Enabled", false };
		ofParameter<int> port{ "Port", 4000 };
		PARAM_DECLARE("OSC", enabled, port);
	} oscParameters;
};