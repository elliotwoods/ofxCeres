#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

#define TOWER_HEIGHT 12
#define IK_SIZE (TOWER_HEIGHT * 2 + 1)

namespace SA = ofxCeres::Models::StructuralAnalysis;

//--------------------------------------------------------------
void ofApp::setup() {
	ofxCeres::Models::StructuralAnalysis::Builder::Chain systemDefinition{
		{
			"Axle"
			, "Left"
			, {
			}
			, {
				"downTop"
				, "downBottom"
			}
		}
		, {
			"TopBlock"
			, "Block"
			, {
				{
					"bearingBottom"
					, "upBottom"
				}, 
				{
					"bearingTop"
					, "upTop"
				}
			}
		}
		, {
			"Axle"
			, "Right"
			, {
				{
					"upTop"
					, "bearing2Top"
				},
				{
					"upBottom"
					, "bearing2Bottom"
				}
			}
			, {
				"downTop"
				, "downBottom"
			}
		}
	};

	ofxCeres::Models::StructuralAnalysis::Builder::FactoryRegister factories;
	factories.addFactory<Data::Block>("Block");
	factories.addFactory<Data::Axle>("Axle");
	factories.addFactory<Data::TopBlock>("TopBlock");
	
	ofxCeres::Models::StructuralAnalysis::Builder::build(this->structuralAnalysis.system
		, systemDefinition
		, factories
	);

	// set the weights
	auto mass = 10.0f; // kg
	this->structuralAnalysis.system.bodies["Block"]->loads["weight"].force = glm::vec3{ 0, -9.81, 0 } * mass;

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
	if (this->structuralAnalysis.solve) {
		this->solve();
	}
	SA::DrawProperties::X().updateMaxScalar();
}

//--------------------------------------------------------------
void ofApp::draw() {

}

//--------------------------------------------------------------
void ofApp::drawWorld() {
	ofDrawGrid(1, 10, true, false, true, false);
	this->structuralAnalysis.system.draw();
}

//--------------------------------------------------------------
void ofApp::solve() {
	auto solverSettings = ofxCeres::Models::StructuralAnalysis::System::getDefaultSolverSettings();
	//solverSettings.options.max_solver_time_in_seconds = this->structuralAnalysis.solveTime;
	try {
		if (this->structuralAnalysis.system.solve<4, 4>(solverSettings)) {
			this->structuralAnalysis.solve = false;
		}
	}
	catch (const ofxCeres::Exception& e) {
		ofLogError() << e.what();
	}
}

//--------------------------------------------------------------
void ofApp::repopulateWidgets() {
	auto inspector = this->widgetsPanel;
	inspector->addFps();
	inspector->addMemoryUsage();
	{
		auto button = inspector->addToggle(this->structuralAnalysis.solve);
		button->setHeight(100.0f);
		button->setHotKey(OF_KEY_RETURN);
	}
	inspector->addButton("Solve once", [this]() {
		this->solve();
	});
	inspector->addSlider(this->structuralAnalysis.solveTime);
	SA::DrawProperties::X().populateInspector(inspector);
	
	this->structuralAnalysis.system.populateInspector(inspector);
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
