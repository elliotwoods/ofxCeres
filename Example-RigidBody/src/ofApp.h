#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void randomizeTransform();
		void solve();

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
		
		vector<glm::vec3> untransformedPoints;
		vector<glm::vec3> transformedPoints;
		glm::mat4 solvedTransform;

		ofEasyCam camera;

		struct {
			ofParameter<float> noise{ "Noise", 0.01, 0, 0.1 };
			ofParameter<bool> randomizeTransform{ "Randomize transform", false };
			ofParameter<bool> solve{ "Solve", false };
		} parameters;
		
		ofxPanel gui;
};
