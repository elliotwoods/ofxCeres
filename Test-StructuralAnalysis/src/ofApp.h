#pragma once

#include "Data/System.h"
#include "ofMain.h"

#define SYSTEM_CONFIG <3, 8>

class ofApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();


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

	
	Data::System SYSTEM_CONFIG system;
};
