#pragma once

#include "ofMain.h"
#include "ofxCvGui.h"

#include "MovingHead.h"

struct CoarseFineValue {
	uint8_t coarse;
	uint8_t fine;

	float getRatio() const {
		auto totalValue = ((uint16_t)coarse << 8) + (uint16_t)fine;
		return (float)totalValue / (float)numeric_limits<uint16_t>::max();
	}
};

struct PanTiltDMX {
	CoarseFineValue pan;
	CoarseFineValue tilt;
};

class ofApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
	void drawWorld();

	void repopulateWidgets();

	void load();
	void save();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
		
	vector<PanTiltDMX> panTiltDMXValues;

	vector<string> names;
	vector<glm::vec2> dmxValues;
	vector<glm::vec2> panTiltAngles;
	vector<glm::vec3> targetPoints;
	vector<ofColor> previewColors;


	glm::mat4 solvedTransform;

	struct {
		ofParameter<glm::vec3> translation{ "Translation", glm::vec3(2.08, 0.78, 4.24), glm::vec3(-10), glm::vec3(+10) };
		ofParameter<glm::vec3> rotation{ "Rotation", glm::vec3(0, -PI/2, 0), glm::vec3(-PI / 2), glm::vec3(+PI / 2) };
		ofParameter<float> tiltOffset{ "Tilt offset", 0, -20, 20 };
		ofParameter<bool> drawGrid{ "Draw grid", true };
		ofParameter<bool> showCursor{ "Show cursor", false };
	} parameters;
		

	map<string, shared_ptr<MovingHead>> movingHeads;
	string selection;

	ofxCvGui::Builder gui;
	shared_ptr<ofxCvGui::Panels::World> worldPanel;
	shared_ptr<ofxCvGui::Panels::Widgets> widgetsPanel;
};
