#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

#include "DMX/FixtureFactory.h"
#include "DMX/Sharpy.h"
#include "DMX/Pointe.h"

//--------------------------------------------------------------
void
ofApp::setup()
{
	// Init the gui
	this->gui.init();

	// Register some fixture types
	DMX::FixtureFactory::X().add<DMX::Sharpy>();
	DMX::FixtureFactory::X().add<DMX::Pointe>();

	// Initialise the scene
	this->scene = Scene::X(); // We want to do this after gui is init setup so we load graphics correctly
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
