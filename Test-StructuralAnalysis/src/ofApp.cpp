#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

//--------------------------------------------------------------
void ofApp::setup() {
	this->system.bodies["Block"] = Data::System SYSTEM_CONFIG::Body{
		{ // loads
			{ //load
				"weight",
				{
					{0, 0, 0}
					, { 0, -9.81 * 100, 0}
				}
			}
		},
		{ // joints
			{
				"bearingUp",
				{
					{-0.5, 0.1, 0}
				}
			},
			{
				"bearingDown",
				{
					{-0.5, 0.1, 0}
				}
			}
		}
	};

	We can do Block, TopBlock, Axle, etc like we did before
		Either they could be classes (need to be templated against SYSTEM_CONFIG)
		Or we could just have functions which make Body objects with the right joints

	Do we need SYSTEM_CONFIG template params on class?
		Or can we just put those onto the solve function?
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

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
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
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
