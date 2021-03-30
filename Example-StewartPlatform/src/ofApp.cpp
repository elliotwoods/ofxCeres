#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"


#define TOWER_HEIGHT 12
#define IK_SIZE (TOWER_HEIGHT * 2 + 1)

namespace SA = ofxCeres::Models::StructuralAnalysis;

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
			this->worldPanel = ofxCvGui::Panels::makeWorld();
			this->worldPanel->onDrawWorld += [this](ofCamera&) {
				this->drawWorld();
			};
			this->worldPanel->setGridEnabled(false);
			{
				auto& camera = this->worldPanel->getCamera();
				camera.setCursorDrawEnabled(true);
				camera.setPosition({ 0, 5, 5 });
				camera.lookAt({ 0, 0, 0 });
			}

			this->worldPanel->onDraw += [this](ofxCvGui::DrawArguments& args) {
				SA::DrawProperties::X().drawScaleLegend(args.localBounds);
			};
			this->stripPanel->add(this->worldPanel);
		}

		// Add the widgets panel
		{
			this->widgetsPanel = ofxCvGui::Panels::makeWidgets();
			this->stripPanel->add(widgetsPanel);
		}

		// Populate the widgets
		this->repopulateWidgets();
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	SA::DrawProperties::X().updateMaxScalar();
	this->stewartPlatform.update();
}

//--------------------------------------------------------------
void ofApp::draw() {

}

//--------------------------------------------------------------
void ofApp::drawWorld() {
	ofDrawGrid(1, 10, true, false, true, false);
	this->stewartPlatform.draw();
}

//--------------------------------------------------------------
void ofApp::solve() {
	this->stewartPlatform.solveForces();
}

//--------------------------------------------------------------
void ofApp::repopulateWidgets() {
	auto inspector = this->widgetsPanel;
	inspector->addFps();
	inspector->addMemoryUsage();
	SA::DrawProperties::X().populateInspector(inspector);
	inspector->addButton("Solve forces", [this]() {
		this->solve();
		});
	inspector->addButton("Solve IK", [this]() {
		this->stewartPlatform.solveIK();
		});
	inspector->addButton("Solve FK", [this]() {
		this->stewartPlatform.solveFK();
		});


	inspector->addParameterGroup(this->stewartPlatform);
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
