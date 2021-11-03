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
						break;
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

//----------
void
Markers::drawWorld()
{
	auto markers = this->getSelection();

	// prepare all glyphs
	map<Marker::Constraint::Options, string> glyphs{
		{ Marker::Constraint::Free, u8"\uF31E"}
		, {Marker::Constraint::Origin, u8"\uf13d"}
		, {Marker::Constraint::Fixed, u8"\uf05b"}
		, {Marker::Constraint::Direction, u8"\uf424"}
	};

	for (auto marker : markers) {
		auto constraint = marker->constraint.get();
		auto glyph = glyphs[marker->constraint.get().get()];

		auto glyphBounds = ofRectangle(0, 0, 25, 25);
		auto textBounds = ofxCvGui::Utils::drawText(marker->name, 25, 5, false, 20, 0, false, ofColor(0), ofxCvGui::getDefaultTypeface(), true);

		auto totalBounds = glyphBounds;
		totalBounds.growToInclude(textBounds);

		ofxCvGui::Utils::drawGraphicAnnotation([marker, glyph, glyphBounds, totalBounds]() {
				// draw contents
				ofxCvGui::Utils::drawGlyph(glyph, glyphBounds);
				ofxCvGui::Utils::drawText(marker->name, 25, 5, false);
			}
			, totalBounds
			, marker->position.get()
			, marker->color.get());
	}
}