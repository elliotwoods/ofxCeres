#pragma once

#include "MovingHead.h"

class Scene : public ofxCvGui::IInspectable {
public:
	Scene();
	void update();
	void drawWorld();
	void renderDMX(vector<uint8_t>&);

	void populateInspector(ofxCvGui::InspectArguments&);
	void load();
	void save();

	map<string, shared_ptr<MovingHead>> & getMovingHeads();
	shared_ptr<MovingHead> addMovingHead(const string & name);
	void addMovingHead(const string& name, shared_ptr<MovingHead>);
	void deleteMovingHead(const string& name);
	void importMovingHead();
protected:
	map<string, shared_ptr<MovingHead>> movingHeads;
	string selection = "";
	ofParameter<bool> drawOtherFixtures{ "Draw other fixtures", true };
};