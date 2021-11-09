#pragma once

#include "ofMain.h"
#include "ofxCvGui.h"

#include "Scene.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	shared_ptr<Scene> scene;

	ofxCvGui::Builder gui;
	shared_ptr<ofxCvGui::Panels::Groups::Strip> stripPanel;
	shared_ptr<ofxCvGui::Panels::WorldManaged> worldPanel;
};
