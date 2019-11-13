#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

//--------------------------------------------------------------
void ofApp::setup() {
	this->system.jointPositionConstraints.resize(2);
	this->system.jointPositionConstraints[0].jointIndex = 3;
	this->system.jointPositionConstraints[0].position = glm::vec2{200, 200};
	this->system.jointPositionConstraints[1].jointIndex = 6;
	this->system.jointPositionConstraints[1].position = glm::vec2{ 400, 400 };

	this->system.bodyLengths.assign(6, 400);
	for (auto & rotation : this->system.currentRotationState) {
		rotation = ofRandom(-PI, 0);
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	if (doSolve) {
		try {
			doSolve = !this->system.solve<6>();
		}
		catch (ofxCeres::Exception e) {
			cout << e.what();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackgroundGradient(40, 10);
	system.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	mousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	if (button == 2) button = 1;
	if (button == 0 || button == 1) {
		this->system.jointPositionConstraints[button].position = glm::vec2(x, y);
	}
	this->doSolve = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
