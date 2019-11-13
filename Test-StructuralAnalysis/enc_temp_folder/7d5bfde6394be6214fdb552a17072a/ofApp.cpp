#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

#define TOWER_HEIGHT 5
#define IK_SIZE (TOWER_HEIGHT * 2 + 1)

namespace SA = ofxCeres::Models::StructuralAnalysis;

//--------------------------------------------------------------
void ofApp::setup() {

	float rotations[IK_SIZE];
	{
		ofxCeres::Models::InverseKinematics::System system;
		
		{
			auto spiral = 0.0f;
			for (auto & rotation : system.currentRotationState) {
				rotation = spiral;
				spiral += 0.2f * PI;
			}
		}

		system.bodyLengths.assign(IK_SIZE, 1.5f);
		system.jointPositionConstraints.push_back(ofxCeres::Models::InverseKinematics::System::JointPositionContraint{
			IK_SIZE - 1
			, glm::vec3 { 8, 0, 0 }
			});

		ofxCeres::SolverSettings solverSettings;
		{
			solverSettings.printReport = true;
			solverSettings.options.minimizer_progress_to_stdout = true;
		}

		system.solve<IK_SIZE>(solverSettings);
		for (int i = 0; i < TOWER_HEIGHT * 2 + 1; i++) {
			rotations[i] = (float) system.currentRotationState[i];
		}
	}

	auto buildTower = [this](const string & towerName, glm::vec3 startPosition, float * rotationsPointer, bool reverseRotationsOrder = false) {
		
		glm::vec3 nextPosition = startPosition;
		float currentRotation = 0.0f;

		for (int i = 0; i < TOWER_HEIGHT; i++) {
			currentRotation += *rotationsPointer;

			auto is = ofToString(i);
			auto ismo = ofToString(i - 1);

			auto blockName = towerName + " Block " + is;
			auto lastBlockName = towerName + " Block " + ismo;
			auto axleName = towerName + " Axle " + is;

			// make the axle
			this->structuralAnalysis.system.bodies[axleName] = make_shared<Data::Axle>();
			auto & axle = this->structuralAnalysis.system.bodies[axleName];
			//axle->rotateRad(currentRotation, glm::vec3(0, 1, 0));
			axle->setPosition(nextPosition);
			if (i == 0) {
				this->structuralAnalysis.system.groundSupports.push_back(SA::System::GroundSupport{
					{axleName, "downTop"}
					});
				this->structuralAnalysis.system.groundSupports.push_back(SA::System::GroundSupport{
					{axleName, "downBottom"}
					});
			}
			else {
				this->structuralAnalysis.system.jointConnections.push_back(SA::System::JointConnection{
					{lastBlockName, "mountUpTop"}
					, {axleName, "downTop"}
					});
				this->structuralAnalysis.system.jointConnections.push_back(SA::System::JointConnection{
					{lastBlockName, "mountUpBottom"}
					, {axleName, "downBottom"}
					});
			}

			// make the block
			this->structuralAnalysis.system.bodies[blockName] = make_shared<Data::Block>();
			auto & block = this->structuralAnalysis.system.bodies[blockName];
			//block->rotateRad(currentRotation, glm::vec3(0, 1, 0));
			block->setPosition(nextPosition);
			this->structuralAnalysis.system.jointConnections.push_back(SA::System::JointConnection{
				{blockName, "bearingTop"}
				, {axleName, "upTop"}
				});
			this->structuralAnalysis.system.jointConnections.push_back(SA::System::JointConnection{
				{blockName, "bearingBottom"}
				, {axleName, "upBottom"}
				});

			// translate to the up top mount
			nextPosition = block->getGlobalTransformMatrix() * block->joints["mountUpTop"].position;
			nextPosition += glm::vec3(0, Data::axleGap, 0);
			
			if (reverseRotationsOrder) {
				rotationsPointer--;
			}
			else {
				rotationsPointer++;
			}
		}
	};

	buildTower("A", glm::vec3{ -4, 0, 0 }, &rotations[0], false);
	buildTower("B", glm::vec3{ 4, 0, 0 }, &rotations[TOWER_HEIGHT * 2], true);


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
			{
				auto & camera = this->worldPanel->getCamera();
				camera.setCursorDrawEnabled(true);
				camera.setPosition({0, 5, 5});
				camera.lookAt({ 0, 0, 0 });
			}

			this->worldPanel->onDraw += [this](ofxCvGui::DrawArguments & args) {
				SA::DrawProperties::X().drawScaleLegend(args.localBounds);
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

	implement IKSolver also


	Handle transforms in the solver (rotations only)
	draw distances between joint connections

	draw JointConnection and GroundSupport objects rather than Joints
	*/

}

//--------------------------------------------------------------
void ofApp::update() {
	if (this->structuralAnalysis.solve) {
		auto solverSettings = ofxCeres::Models::StructuralAnalysis::System::getDefaultSolverSettings();
		//solverSettings.options.max_solver_time_in_seconds = this->structuralAnalysis.solveTime;
		if (this->structuralAnalysis.system.solve<(TOWER_HEIGHT * 4 - 2) * 2, 2 * 2>(solverSettings)) {
			this->structuralAnalysis.solve = false;
		}
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
void ofApp::repopulateWidgets() {
	auto inspector = this->widgetsPanel;
	inspector->addFps();
	inspector->addMemoryUsage();
	{
		auto button = inspector->addToggle(this->structuralAnalysis.solve);
		button->setHeight(100.0f);
		button->setHotKey(OF_KEY_RETURN);
	}
	inspector->addSlider(this->structuralAnalysis.solveTime);
	SA::DrawProperties::X().populateInspector(inspector);
	
	{
		inspector->addTitle("System");
		for (auto & body : this->structuralAnalysis.system.bodies) {
			inspector->addTitle(body.first, ofxCvGui::Widgets::Title::H2);

			//inspector->addTitle("Draw", ofxCvGui::Widgets::Title::H3);
			//{
			//	inspector->addToggle(body.second.drawArgs.enabled);
			//	inspector->addToggle(body.second.drawArgs.joints);
			//	inspector->addToggle(body.second.drawArgs.loads);
			//}
		}
	}
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
