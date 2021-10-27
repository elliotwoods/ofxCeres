#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

//--------------------------------------------------------------
void ofApp::setup() {
	// Initialise the gui
	{
		this->gui.init();

		this->stripPanel = ofxCvGui::Panels::Groups::makeStrip();
		{
			this->stripPanel->setCellSizes({ -1, 400 });
			this->gui.add(this->stripPanel);
		}

		// Add the panel for drawing 3D world
		{
			this->worldPanel = ofxCvGui::Panels::makeWorldManaged();
			this->worldPanel->onDrawWorld += [this](ofCamera &) {
				this->drawWorld();
			};
			this->stripPanel->add(this->worldPanel);
		}

		// Add the inspector and focus this to start with
		{
			auto inspector = ofxCvGui::Panels::makeInspector();
			inspector->setTitleEnabled(false);
			this->stripPanel->add(inspector);
			ofxCvGui::inspect(this->scene);
		}
	}

	// Load for json file
	this->scene->load();

	// look at moving head #1 to start with
	{
		auto& movingHeads = this->scene->getMovingHeads();
		if (!movingHeads.empty()) {
			auto position4 = movingHeads.begin()->second->getTransform() * glm::vec4(0, 0, 0, 1);
			auto position = (glm::vec3)(position4 / position4.w);
			this->worldPanel->getCamera().lookAt(position);
		}
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	this->renderDMX();
	this->scene->update();
}

//--------------------------------------------------------------
void ofApp::draw() {

}

//--------------------------------------------------------------
void ofApp::drawWorld() {
	this->scene->drawWorld();
}

//--------------------------------------------------------------
void ofApp::renderDMX() {
	// dmx values have addersses starting with 1, so we keep these addresses and throw away the first value
	vector<uint8_t> dmxValues(513, 0);
	this->scene->renderDMX(dmxValues);

	//--
	// HERE YOU NEED TO SEND DMX OUT
	//--
	//
	//

	//
	//--
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
