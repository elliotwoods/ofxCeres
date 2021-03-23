#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

#define TOWER_HEIGHT 12
#define IK_SIZE (TOWER_HEIGHT * 2 + 1)

namespace SA = ofxCeres::Models::StructuralAnalysis;

//--------------------------------------------------------------
void ofApp::setup() {
	float rotations[IK_SIZE];
	{
		{
			auto spiral = 0.0f;
			for (auto & rotation : this->inverseKinematics.system.currentRotationState) {
				rotation = spiral;
				spiral += 0.2f * PI;
			}
		}

		this->inverseKinematics.system.bodyLengths.assign(IK_SIZE, 1.0f);
		this->inverseKinematics.system.jointPositionConstraints.push_back(ofxCeres::Models::InverseKinematics::System::JointPositionContraint{
			IK_SIZE
			, glm::vec3 { 8, 0, 0 }
			});
		this->inverseKinematics.system.domainConstraints.push_back({
			{
				4 - (12.7 / 2), -5.7 / 2
				, 12.7, 5.7
			}
			});

		auto solverSettings = ofxCeres::Models::InverseKinematics::System::getDefaultSolverSettings();
		{
			solverSettings.printReport = true;
			solverSettings.options.minimizer_progress_to_stdout = true;
		}

		this->inverseKinematics.system.solve<IK_SIZE>(solverSettings);
		for (int i = 0; i < TOWER_HEIGHT * 2 + 1; i++) {
			rotations[i] = (float)this->inverseKinematics.system.currentRotationState[i];
		}
	}

	ofxCeres::Models::StructuralAnalysis::Builder::Chain systemDefinition;

	auto getNextRotate = [&]() {
		static float * movingRotation = rotations;
		static float totalRotation = 0;
		totalRotation += *movingRotation++;
		return glm::angleAxis(totalRotation, glm::vec3{ 0,1,0 });
	};

	// TOWER UP
	auto movingRotation = rotations;
	int layerIndex = 0;
	for (int i = 0; i < TOWER_HEIGHT; i++) {
		systemDefinition.push_back({
			"Axle"
			, "A " + ofToString(i) + " Axle"
			});
		if (i == 0) {
			systemDefinition.back().groundSupports.push_back("downTop");
			systemDefinition.back().groundSupports.push_back("downBottom");
		}
		else {
			systemDefinition.back().jointConnections.push_back({ "downTop", "mountUpTop" });
			systemDefinition.back().jointConnections.push_back({ "downBottom", "mountUpBottom" });
		}

		systemDefinition.push_back({
			"Block"
			, "A " + ofToString(i) + " Block"
			, {
				{"bearingBottom", "upBottom"}
				, {"bearingTop", "upTop"}
			}
			});
		systemDefinition.back().orientation = getNextRotate();
	}
	systemDefinition.push_back({
		"Axle", "A Top Axle"
		, {
			{ "downTop", "mountUpTop"}
			, { "downBottom", "mountUpBottom"}
		}
		});
	systemDefinition.push_back({
		"TopBlock", "TopBlock"
		, {
			{"bearingTop", "upTop"}
			, {"bearingBottom", "upBottom"}
		}
		});
	systemDefinition.back().orientation = getNextRotate();
	systemDefinition.push_back({
		"Axle", "B Top Axle"
		, {
			{ "upTop", "bearing2Top"}
			, { "upBottom", "bearing2Bottom"}
		}
		});
	auto flipBlock = glm::rotate(glm::quat{}, (float)PI, glm::vec3(0, 1, 0));

	for (int i = 0; i < TOWER_HEIGHT; i++) {
		systemDefinition.push_back({
			"Block"
			, "B " + ofToString(i) + " Block"
			, {
				{"mountUpTop", "downTop"}
				, {"mountUpBottom", "downBottom" }
			}
		});
		systemDefinition.back().orientation = getNextRotate() * flipBlock;

		systemDefinition.push_back({
			"Axle"
			, "B " + ofToString(i) + " Axle"
			, {
				{"upTop", "bearingTop"}
				, {"upBottom", "bearingBottom"}
			}
		});

		if (i == TOWER_HEIGHT - 1) {
			systemDefinition.back().groundSupports.push_back("downTop");
			systemDefinition.back().groundSupports.push_back("downBottom");
		}
	}

	ofxCeres::Models::StructuralAnalysis::Builder::FactoryRegister factories;
	factories.addFactory<Data::Block>("Block");
	factories.addFactory<Data::Axle>("Axle");
	factories.addFactory<Data::TopBlock>("TopBlock");
	
	ofxCeres::Models::StructuralAnalysis::Builder::build(this->structuralAnalysis.system
		, systemDefinition
		, factories
	);

	// set the weights
	for (auto bodyIt : this->structuralAnalysis.system.bodies) {
		auto nameParts = ofSplitString(bodyIt.first, " ");
		if (nameParts.size() == 3 && nameParts[2] == "Block") {
			auto layerIndex = ofToInt(nameParts[1]);
			auto mass = Data::layerWeights[layerIndex];
			bodyIt.second->loads["weight"].force = glm::vec3{ 0, -9.81, 0 } * mass;
		}
	}
	{
		auto topBlock = this->structuralAnalysis.system.bodies["TopBlock"];
		topBlock->loads["weight"].force = glm::vec3{ 0, -9.81, 0 } * Data::layerWeights[TOWER_HEIGHT];
	}

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

	{
		auto PandRs = this->inverseKinematics.system.getCurrentPositionsAndRotations();
		ofPolyline line;
		for (const auto& PandR : PandRs) {
			line.addVertex(glm::vec3{ PandR.position.x, 0, -PandR.position.y });
		}
		line.draw();
	}
	{
		ofPushStyle();
		{
			ofNoFill();
			auto& domain = this->inverseKinematics.system.domainConstraints[0].domain;
			ofRotateDeg(-90, 1, 0, 0);
			ofDrawRectangle(domain);
		}
		ofPopStyle();
	}
}

//--------------------------------------------------------------
void ofApp::solve() {
	auto solverSettings = ofxCeres::Models::StructuralAnalysis::System::getDefaultSolverSettings();
	//solverSettings.options.max_solver_time_in_seconds = this->structuralAnalysis.solveTime;
	try {
		if (this->structuralAnalysis.system.solve<(TOWER_HEIGHT * 4 - 2) * 2 + 8, 2 * 2>(solverSettings)) {
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
