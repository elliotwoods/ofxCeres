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
				camera.setPosition({ 3, 3, 3 });
				camera.lookAt({ 0, 0.5, 0 });
				camera.setFov(20);
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


	// load last save
	{
		if (ofFile::doesFileExist(LAST_SAVE_PATH)) {
			std::string filePath;
			{
				auto file = ofFile(LAST_SAVE_PATH);
				file >> filePath;
				file.close();
			}

			if (ofFile::doesFileExist(filePath)) {
				this->load(filePath);
			}
		}
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
	inspector->addButton("Load", [this] {
		this->load();
	});
	inspector->addButton("Save", [this] {
		this->save();
	});
	inspector->addLiveValue(this->lastFilePath);

	SA::DrawProperties::X().populateInspector(inspector);

	auto drawSolved = [](const ofxCvGui::DrawArguments& drawArgs, bool solved) {
		ofPushStyle();
		if (solved) {
			ofFill();
		}
		else {
			ofNoFill();
		}
		ofDrawCircle(20
			, drawArgs.localBounds.height / 2.0f
			, 7.0f);
		ofPopStyle();
	};

	inspector->addButton("Solve forces", [this]() {
		this->solve();
		})->onDraw += [this, drawSolved](const ofxCvGui::DrawArguments & drawArgs) {
			drawSolved(drawArgs, this->stewartPlatform.isForcesSolved());
		};
	inspector->addButton("Solve IK", [this]() {
		this->stewartPlatform.solveIK();
		});
	inspector->addButton("Solve FK", [this]() {
		this->stewartPlatform.solveFK();
		})->onDraw += [this, drawSolved](const ofxCvGui::DrawArguments& drawArgs) {
			drawSolved(drawArgs, this->stewartPlatform.isFKSolved());
		};


	inspector->addParameterGroup(this->stewartPlatform);
}

//--------------------------------------------------------------
void ofApp::load() {
	auto result = ofSystemLoadDialog("Save configuration");
	if (result.bSuccess) {
		this->load(result.filePath);
	}
}

//--------------------------------------------------------------
void ofApp::load(const std::string& path) {
	nlohmann::json json;
	ofFile file(path, ofFile::ReadOnly);
	file >> json;
	file.close();
	this->stewartPlatform.deserialize(json);
	this->lastFilePath = path;
}
//--------------------------------------------------------------
void ofApp::save() {
	auto result = ofSystemSaveDialog("stewartPlatform.json", "Save configuration");
	if (result.bSuccess) {
		{
			nlohmann::json json;
			this->stewartPlatform.serialize(json);
			ofFile file(result.filePath, ofFile::WriteOnly);
			file << json;
			file.close();
		}
		
		{
			ofFile file(LAST_SAVE_PATH, ofFile::WriteOnly);
			file << result.filePath;
			file.close();
		}

		this->lastFilePath = result.filePath;
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
