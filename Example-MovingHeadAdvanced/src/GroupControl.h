#pragma once

#include "Data/Serializable.h"
#include "OSC/Router.h"
#include "ofxCvGui.h"

class GroupControl : public Data::Serializable, public OSC::Router, public ofxCvGui::IInspectable {
public:
	GroupControl();
	string getTypeName() const;

	void init();
	void update();
	void drawWorld();

	void serialize(nlohmann::json&);
	void deserialize(const nlohmann::json&);
	void populateInspector(ofxCvGui::InspectArguments&);

	void navigateTo(const glm::vec3&);
protected:
	bool initialised = false;
	deque<glm::vec3> history;
	bool needsWorldUpdate = true;

	struct : ofParameterGroup {
		ofParameter<bool> trackCursor{ "Track cursor", false };

		struct : ofParameterGroup {
			struct : ofParameterGroup {
				ofParameter<bool> enabled{ "Enabled", true };
				ofParameter<int> length{ "Length", 100, 0, 1000 };
				PARAM_DECLARE("History trail", enabled, length);
			} historyTrail;
			PARAM_DECLARE("Draw", historyTrail);
		} draw;
		
		PARAM_DECLARE("Group Control", trackCursor, draw);
	} parameters;
};