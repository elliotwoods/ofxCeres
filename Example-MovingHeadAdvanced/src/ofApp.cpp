#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

#include "DMX/FixtureFactory.h"
#include "DMX/Sharpy.h"

//--------------------------------------------------------------
void
ofApp::setup()
{
	// Init the gui
	this->gui.init();

	// Register some fixture types
	DMX::FixtureFactory::X().add<DMX::Sharpy>();

	// Initialise the scene
	this->scene = make_shared<Scene>(); // We want to do this after gui is init setup so we load graphics correctly
	this->scene->load(Scene::getDefaultFilename());

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
			auto position = movingHeads.begin()->second->getModel()->getPosition();
			auto worldPanel = this->scene->getPanel();
			worldPanel->getCamera().lookAt(position);
		}
	}
}

//--------------------------------------------------------------
void
ofApp::update()
{
	this->scene->update();
}

//--------------------------------------------------------------
void
ofApp::draw()
{

}
