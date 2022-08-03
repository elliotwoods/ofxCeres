#pragma once

#include "ofMain.h"
#include "ofxCvGui.h"

#include "Scene.h"
#include "oneTower.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    void drawWorld();
    
    void keyPressed(int key);
    void keyReleased (int key);
    
	shared_ptr<Scene> scene;

	ofxCvGui::Builder gui;
	shared_ptr<ofxCvGui::Panels::Groups::Strip> stripPanel;
    
    vector<oneTower> allTowers;
    
    
};
