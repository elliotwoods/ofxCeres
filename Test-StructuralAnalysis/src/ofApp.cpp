#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

//--------------------------------------------------------------
void ofApp::setup() {
	this->system.bodies["Block"] = Data::Block();
	this->system.bodies["Axle 1"] = Data::Axle();

	this->system.jointConnections.push_back(Data::System::JointConnection {
		{"Block", "bearingUp"},
		{"Axle 1", "upTop"}
		});
	this->system.jointConnections.push_back(Data::System::JointConnection{
		{"Block", "bearingDown"},
		{"Axle 1", "upBottom"}
		});
	this->system.groundSupports.push_back(Data::System::GroundSupport{
		{"Axle 1", "downTop"}
		});
	this->system.groundSupports.push_back(Data::System::GroundSupport{
		{"Axle 1", "downBottom"}
		});
	this->system.solve<2, 2>();

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
			this->worldPanel->onDrawWorld += [this](ofCamera &) {
				this->drawWorld();
			};
			this->worldPanel->setGridEnabled(false);
			this->worldPanel->getCamera().setCursorDrawEnabled(true);

			this->worldPanel->onDraw += [this](ofxCvGui::DrawArguments & args) {
				Data::DrawProperties::X().drawScaleLegend(args.localBounds);
			};
			this->stripPanel->add(this->worldPanel);
		}

		// Add the widgets panel
		{
			this->widgetsPanel = ofxCvGui::Panels::makeWidgets();
			this->stripPanel->add(widgetsPanel);
		}

		// Popualte the widgets
		this->repopulateWidgets();
	}

	/*
	Draw with colours
		(large forces get brighter)
		body can be less white

	try a more complex system
	*/

}

//--------------------------------------------------------------
void ofApp::update() {
	Data::DrawProperties::X().updateMaxScalar();
}

//--------------------------------------------------------------
void ofApp::draw() {

}

//--------------------------------------------------------------
void ofApp::drawWorld() {
	ofDrawGrid(1, 10, true, false, true, false);
	this->system.draw();
}

//--------------------------------------------------------------
void ofApp::repopulateWidgets() {
	auto inspector = this->widgetsPanel;
	inspector->addFps();
	inspector->addMemoryUsage();
	Data::DrawProperties::X().populateInspector(inspector);
	this->system.populateInspector(inspector);
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
