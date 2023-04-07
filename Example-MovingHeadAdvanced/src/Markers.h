#pragma once

#include "Data/CalibrationPointSet.h"
#include "MarkerInWorld.h"
#include "Mesh.h"

class Markers : public Data::CalibrationPointSet<MarkerInWorld> {
public:
	Markers(shared_ptr<Mesh>, shared_ptr<ofxCvGui::Panels::WorldManaged>);

	shared_ptr<MarkerInWorld> getMarkerClosestTo(const glm::vec3&);
	shared_ptr<MarkerInWorld> getMarkerByName(const string&);
	shared_ptr<MarkerInWorld> addNewMarker(const string& name
		, const glm::vec3& position
		, bool useExistingIfWeHaveAMatch);

	void drawWorld();
	shared_ptr<MarkerInWorld> addMarker();

	void populateInspector(ofxCvGui::InspectArguments&) override;
protected:
	shared_ptr<Mesh> mesh;
	shared_ptr<ofxCvGui::Panels::WorldManaged> worldPanel;

	ofParameter<bool> snapToVertex{ "Snap to vertex", false };

	glm::vec3 cursorPosition;
};
