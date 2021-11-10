#pragma once

#include "Data/CalibrationPointSet.h"
#include "Marker.h"
#include "Mesh.h"

class Markers : public Data::CalibrationPointSet<Marker> {
public:
	Markers(shared_ptr<Mesh>, shared_ptr<ofxCvGui::Panels::WorldManaged>);

	shared_ptr<Marker> getMarkerClosestTo(const glm::vec3&);
	shared_ptr<Marker> getMarkerByName(const string&);
	shared_ptr<Marker> addNewMarker(const string& name
		, const glm::vec3& position
		, bool useExistingIfWeHaveAMatch);

	void drawWorld();
	void addMarker();

	void populateInspector(ofxCvGui::InspectArguments&) override;
protected:
	shared_ptr<Mesh> mesh;
	shared_ptr<ofxCvGui::Panels::WorldManaged> worldPanel;

	glm::vec3 cursorPosition;
};