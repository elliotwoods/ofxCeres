#pragma once

#include "ofMain.h"
#include "ofxCvGui.h"
#include "ofxCeres.h"
#include "Data/StewartPlatform.h"
#include "Procedure/SearchPlane.h"
#include "ofxDualSense.h"
#include "ofxOsc.h"

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
	Procedure::SearchPlane searchPlane;

	ofParameter<string> lastFilePath{ "Last file path", "" };

	std::vector<std::shared_ptr<ofxDualSense::Controller>> controllers;
	ofParameter<float> movementSpeed{ "Movement speed [/s]", 0.05, 0, 1 };

	struct OscParameters : ofParameterGroup {
		ofParameter<bool> enabled{ "Enabled", 7777 };
		ofParameter<int> port{ "Port", 7777 };
		ofParameter<string> host{ "Host", "localhost" };
		OscParameters() {
			this->setName("OscParameters");
			this->add(this->enabled);
			this->add(this->port);
			this->add(this->host);
		}
	} oscParameters;

	struct PayloadParameters : ofParameterGroup {
		ofParameter<bool> draw{ "Draw", true };
		ofParameter<float> diameter{ "Diameter", 1.8, 0, 2 };
		ofParameter<float> offset{ "Offset", 0.12, 0, 1 };
		PayloadParameters() {
			this->setName("Payload");
			this->add(this->draw);
			this->add(this->diameter);
			this->add(this->offset);
		}
	} payloadParameters;

	shared_ptr<ofxOscSender> oscSender;
};
