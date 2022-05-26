#pragma once

#include "Data/CalibrationPointSet.h"
#include "ofxMarker.h"
#include "Mesh.h"

class Markers : public Data::CalibrationPointSet<ofxMarker> {
public:
	Markers(shared_ptr<Mesh>, shared_ptr<ofxCvGui::Panels::WorldManaged>);

	shared_ptr<ofxMarker> getMarkerClosestTo(const glm::vec3&);
	shared_ptr<ofxMarker> getMarkerByName(const string&);
	shared_ptr<ofxMarker> addNewMarker(const string& name
		, const glm::vec3& position
		, bool useExistingIfWeHaveAMatch);

	void drawWorld();
	shared_ptr<ofxMarker> addMarker();

	void populateInspector(ofxCvGui::InspectArguments&) override;
protected:
	shared_ptr<Mesh> mesh;
	shared_ptr<ofxCvGui::Panels::WorldManaged> worldPanel;

	ofParameter<bool> snapToVertex{ "Snap to vertex", false };

	glm::vec3 cursorPosition;
};
