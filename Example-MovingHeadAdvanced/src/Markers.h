#pragma once

#include "Data/CalibrationPointSet.h"
#include "Marker.h"

class Markers : public Data::CalibrationPointSet<Marker> {
public:
	void focusMarkerClosestTo(const glm::vec3&);
	shared_ptr<Marker> getMarkerByName(const string&);
	shared_ptr<Marker> getFocusedMarker() const;
	shared_ptr<Marker> addNewMarker(const string& name
		, const glm::vec3& position
		, bool useExistingIfWeHaveAMatch);
protected:
	shared_ptr<Marker> focusedMarker;
};