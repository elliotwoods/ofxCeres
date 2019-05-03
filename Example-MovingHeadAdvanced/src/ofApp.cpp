#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

//--------------------------------------------------------------
void ofApp::setup() {

	// Initialise the moving heads
	{
		this->movingHeads.emplace("1", make_shared<MovingHead>());
		this->movingHeads.emplace("2", make_shared<MovingHead>());
	}
	this->selection = this->movingHeads.begin()->first;


	// Initialise the gui
	{
		this->gui.init();

		this->stripPanel = ofxCvGui::Panels::Groups::makeStrip();
		{
			this->stripPanel->setCellSizes({ -1, 400, 350 });
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
			this->stripPanel->add(this->worldPanel);

		}

		// Add the widgets panel
		{
			this->widgetsPanel = ofxCvGui::Panels::makeWidgets();
			this->stripPanel->add(widgetsPanel);
		}

		// Add a blank panel in this slot for now (this will become the list panel)
		{
			this->listPanelHolder = make_shared<ofxCvGui::Panels::Groups::Grid>();
			this->listPanelHolder->add(this->movingHeads[selection]->getListPanel());

			this->stripPanel->add(listPanelHolder);
		}

		// Popualte the widgets
		this->repopulateWidgets();
	}

	// Load for json file
	this->load();

	// look at moving head #1 to start with
	{
		auto position4 = this->movingHeads.begin()->second->getTransform() * glm::vec4(0, 0, 0, 1);
		auto position = (glm::vec3) (position4 / position4.w);
		this->worldPanel->getCamera().lookAt(position);
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	for (const auto & movingHead : this->movingHeads) {
		movingHead.second->update();
	}

	this->renderDMX();
}

//--------------------------------------------------------------
void ofApp::draw() {

}

//--------------------------------------------------------------
void ofApp::drawWorld() {

	// Draw a floor grid
	if (this->drawGrid) {
		ofPushStyle();
		{
			ofSetColor(150);

			ofPushMatrix();
			{
				ofRotateDeg(180, 0, 1, 0);
				ofRotateDeg(-90, 0, 0, 1);
				ofDrawGridPlane(1.0f, 20, true);
			}
			ofPopMatrix();
		}
		ofPopStyle();
	}

	// Draw the moving heads
	for (auto & movingHead : this->movingHeads) {
		movingHead.second->drawWorld(movingHead.first == this->selection);
	}
}

//--------------------------------------------------------------
void ofApp::renderDMX() {
	// dmx values have addersses starting with 1, so we keep these addresses and throw away the first value
	vector<uint8_t> dmxValues(513, 0);

	for (auto & movingHead : this->movingHeads) {
		movingHead.second->renderDMX(dmxValues);
	}

	//--
	// HERE YOU NEED TO SEND DMX OUT
	//--
	//
	//

	//
	//--
}

//--------------------------------------------------------------
void ofApp::repopulateWidgets() {
	this->widgetsPanel->clear();

	static shared_ptr<ofxCvGui::Widgets::MultipleChoice> selector;
	if (!selector) {
		selector = make_shared<ofxCvGui::Widgets::MultipleChoice>("Moving head");
		{
			for (auto & it : this->movingHeads) {
				selector->addOption(it.first);
			}
		}

		selector->onValueChange += [this](int) {
			this->selection = selector->getSelection();
			this->repopulateWidgets();

			// bring up the list in that panel slot
			this->listPanelHolder->clear();
			this->listPanelHolder->add(this->movingHeads[selection]->getListPanel());
		};
	}

	this->widgetsPanel->addFps();
	this->widgetsPanel->addButton("Save all", [this]() {
		this->save();
	});
	this->widgetsPanel->add(selector);

	this->widgetsPanel->addSpacer();

	this->movingHeads[this->selection]->populateWidgets(this->widgetsPanel);

	this->widgetsPanel->addSpacer();

	this->widgetsPanel->addToggle(this->drawGrid);
}

//--------------------------------------------------------------
void ofApp::load() {
	for (const auto & it : this->movingHeads) {
		ofFile file;
		file.open(it.first + ".json");
		if (file.exists()) {
			nlohmann::json json;
			file >> json;
			it.second->deserialize(json);
		}
	}
}

//--------------------------------------------------------------
void ofApp::save() {
	for (const auto & it : this->movingHeads) {
		nlohmann::json json;
		it.second->serialize(json);

		ofFile file;
		file.open(it.first + ".json", ofFile::Mode::WriteOnly);
		file << std::setw(4) << json;
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
	this->movingHeads[this->selection]->setWorldCursorPosition(this->worldPanel->getCamera().getCursorWorld());
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
