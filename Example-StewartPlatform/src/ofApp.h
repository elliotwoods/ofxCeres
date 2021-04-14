#pragma once

#include "ofMain.h"
#include "ofxCvGui.h"
#include "ofxCeres.h"
#include "Data/StewartPlatform.h"
#include "ofxDualSense.h"

#define LAST_SAVE_PATH "lastSave.txt"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void drawWorld();
	void solve();
	void repopulateWidgets();

	void load();
	void load(const std::string &);
	void save();
	void setLastFilePath(const std::string&);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ofxCvGui::Builder gui;
	shared_ptr<ofxCvGui::Panels::Groups::Strip> stripPanel;
	shared_ptr<ofxCvGui::Panels::WorldManaged> worldPanel;
	shared_ptr<ofxCvGui::Panels::Widgets> widgetsPanel;

	Data::StewartPlatform stewartPlatform;

	ofParameter<string> lastFilePath{ "Last file path", "" };

	std::vector<std::shared_ptr<ofxDualSense::Controller>> controllers;
	ofParameter<float> movementSpeed{ "Movement speed [/s]", 0.1, 0, 1 };
};
