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
        
        //this is needed to get access and add to the 3d world drawing
        this->scene->getPanel()->onDrawWorld += [this](ofCamera&) {
            this->drawWorld();
        };
        
        
        // Add the inspector and focus this to start with
        {
            auto inspector = ofxCvGui::Panels::makeInspector();
            inspector->setTitleEnabled(false);
            this->stripPanel->add(inspector);
            ofxCvGui::inspect(this->scene);
        }
    }
    
    allTowers.resize(3);
    
    allTowers[0].setup(0, "north");
    allTowers[1].setup(1, "way");
    allTowers[2].setup(2, "forest");
    
    
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

void ofApp::drawWorld() {
    ofPushStyle();
    //    ofLog()<<"drawWorld() ";
    for(auto & aTower : allTowers){
        aTower.draw();
    }
    ofSetColor(ofColor::red);
    ofDrawSphere(ofMap(mouseX,0,ofGetWidth(),10,-10), 3, ofMap(mouseY,0,ofGetHeight(),10,-10), 0.3);
    ofPopStyle();
}

void ofApp::keyPressed(int key){
    
}

void ofApp::keyReleased(int key) {
    
}
