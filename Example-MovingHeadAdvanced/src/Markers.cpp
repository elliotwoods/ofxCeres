#include "pch_ofApp.h"
#include "Markers.h"

//----------
void
Markers::focusMarkerClosestTo(const glm::vec3& position)
{
	// Focus the data point close to the world cursor
	auto minDistance2 = numeric_limits<float>::max();
	auto markers = this->getSelection();
	for (auto marker : markers) {
		auto distance2 = glm::distance2(marker->position.get(), position);

		// take the point as the focus if it's closest to the cursor so far (within 30cm)
		if (distance2 < minDistance2 && sqrt(distance2) < 0.3f) {
			this->focusedMarker = marker;
			minDistance2 = distance2;
		}
	}
}

//----------
shared_ptr<Marker>
Markers::getMarkerByName(const string& name)
{
	auto selection = this->getSelection();
	for (auto marker : selection) {
		if (marker->name.get() == name) {
			return marker;
		}
	}

	// None found. Return empty
	return shared_ptr<Marker>();
}

//----------
shared_ptr<Marker>
Markers::getFocusedMarker() const
{
	return this->focusedMarker;
}

//----------
shared_ptr<Marker>
Markers::addNewMarker(const string& name, const glm::vec3& position, bool useExistingIfWeHaveAMatch)
{
	// Find if there's an existing marker which matches this description
	auto priorMarkers = this->getAllCaptures();
	bool foundPriorMarker = false;
	shared_ptr<Marker> marker;
	for (const auto& priorMarker : priorMarkers) {
		if (priorMarker->name.get() == name) {
			// There's a marker in our a database with a matching name
			// Check if its position data is the same
			if (priorMarker->position.get() == position) {
				// OK total match - just use this one
				marker = priorMarker;
			}
			else {
				// Marker with existing name exists but position is different
				// Find a new name which is free
				for (int index = 2; ; index++) {
					bool priorWithTransformedNameExists = false;
					string transformedName = name + "-" + ofToString(index);
					for (const auto& priorMarker2 : priorMarkers) {
						if (priorMarker2->name.get() == transformedName) {
							priorWithTransformedNameExists = true;
							break;
						}
					}
					if (!priorWithTransformedNameExists) {
						marker = make_shared<Marker>();
						marker->name.set(transformedName);
						marker->position.set(position);
					}
				}
			}
			break;
		}
	}

	// Make the marker if there is no prior
	if (!marker) {
		marker = make_shared<Marker>();
		marker->position.set(position);
		marker->name.set(name);
	}
	
	return marker;
}