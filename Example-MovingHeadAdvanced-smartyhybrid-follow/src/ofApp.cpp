#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

#include "DMX/FixtureFactory.h"
#include "DMX/Sharpy.h"
#include "DMX/Pointe.h"
#include "DMX/SmartyHybrid.h"

//--------------------------------------------------------------
void
ofApp::setup()
{
	// Init the gui
	this->gui.init();

	// Register some fixture types
	DMX::FixtureFactory::X().add<DMX::Sharpy>();
	DMX::FixtureFactory::X().add<DMX::Pointe>();
    DMX::FixtureFactory::X().add<DMX::SmartyHybrid>();
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
    
    allTowers.resize(3);
    auto& movingHeads =scene->getMovingHeads();
    for ( auto& head : movingHeads) {
        auto nameParts = ofSplitString(head.first, "-", true, true);
        
        bool bValidName = false;
        if(nameParts.size() >= 2){
            int towerNumber = nameParts[0][0] - 'a';
            if(towerNumber >= 0  &&  towerNumber < 3){
                int fixtureNum = ofToInt(nameParts[1]);
                if(fixtureNum >= 0 && fixtureNum < 9){
                    bValidName = true;
                    allTowers[towerNumber].addMovingHead(ElbSleketonParts(fixtureNum), head.second);
                }
            }
        }
        
        if(!bValidName){
        
            ofLogWarning("ofApp::setup") << "invalid name for assigning to tower";
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

void ofApp::keyPressed(int key){
    
}

void ofApp::keyReleased(int key) {
    
}
