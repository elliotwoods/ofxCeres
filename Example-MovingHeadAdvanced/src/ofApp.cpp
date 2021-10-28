#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

//--------------------------------------------------------------
void
ofApp::setup()
{
	// Init the gui
	this->gui.init();

	// Initialise the scene
	this->scene = make_shared<Scene>(); // We want to do this after gui is init setup so we load graphics correctly
	this->scene->load();

	// Setup the gui
	{
		this->gui.init();

		this->stripPanel = ofxCvGui::Panels::Groups::makeStrip();
		{
			this->stripPanel->setCellSizes({ -1, 400 });
			this->gui.add(this->stripPanel);
		}

		// Add the panel for drawing 3D world
		{
			this->stripPanel->add(this->scene->getPanel());
		}

		// Add the inspector and focus this to start with
		{
			auto inspector = ofxCvGui::Panels::makeInspector();
			inspector->setTitleEnabled(false);
			this->stripPanel->add(inspector);
			ofxCvGui::inspect(this->scene);
		}
	}

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
void
ofApp::update()
{
	this->renderDMX();
	this->scene->update();
}

//--------------------------------------------------------------
void
ofApp::draw()
{

}

//--------------------------------------------------------------
void
ofApp::renderDMX()
{
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