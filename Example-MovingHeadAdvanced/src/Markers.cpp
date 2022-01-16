#include "pch_ofApp.h"
#include "Markers.h"
#include "Scene.h"

//----------
Markers::Markers(shared_ptr<Mesh> mesh, shared_ptr<ofxCvGui::Panels::WorldManaged> worldPanel)
	: mesh(mesh)
	, worldPanel(worldPanel)
{
	{
		auto worldPanelWeak = weak_ptr<ofxCvGui::Element>(worldPanel);
		worldPanel->onMouse += [this, worldPanelWeak](ofxCvGui::MouseArguments& args) {
			auto worldPanel = worldPanelWeak.lock();
			if (this->isBeingInspected() && worldPanel->isMouseOver()) {
				auto cursorPosition = this->worldPanel->getCamera().getCursorWorld();
				if (this->snapToVertex.get()) {
					this->cursorPosition = this->mesh->getPointClosestTo(cursorPosition, 0.3f);
				}
				else {
					this->cursorPosition = cursorPosition;
				}
			}
		};
	}

	this->onSerialize += [this](nlohmann::json& json) {
		json << this->snapToVertex;
	};
	this->onDeserialize += [this](const nlohmann::json& json) {
		json >> this->snapToVertex;
	};
}

//----------
shared_ptr<Marker>
Markers::getMarkerClosestTo(const glm::vec3& position)
{
	auto minDistance2 = numeric_limits<float>::max();
	auto markers = this->getSelection();
	shared_ptr<Marker> closestMarker;
	for (auto marker : markers) {
		auto distance2 = glm::distance2(marker->position.get(), position);

		if (distance2 < minDistance2 && sqrt(distance2) < 0.3f) {
			closestMarker = marker;
			minDistance2 = distance2;
		}
	}
	return closestMarker;
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
		this->add(marker);
	}
	
	return marker;
}

//----------
void
Markers::drawWorld()
{
	auto markers = this->getSelection();

	for (auto marker : markers) {
		auto constraint = marker->constraint.get();

		// Check if constraint is free or other
		if (constraint == Marker::Constraint::Free) {
			// Draw using normal text annotation
			ofxCvGui::Utils::drawTextAnnotation(marker->name.get()
				, marker->position.get()
				, marker->color.get());
		}
		else {
			// Draw with glyph
			auto glyph = Marker::getGlyphForConstraint(marker->constraint.get());

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

	if (this->isBeingInspected()) {
		ofxCvGui::Utils::drawTextAnnotation(ofToString(this->cursorPosition, 3)
			, this->cursorPosition);
	}
}

//----------
shared_ptr<Marker>
Markers::addMarker()
{
	auto name = ofSystemTextBoxDialog("Marker name");
	if (name.empty()) {
		return shared_ptr<Marker>();
	}

	shared_ptr<Marker> marker;

	// check if existing marker exists
	{
		auto markers = this->getAllCaptures();
		for (auto priorMarker : markers) {
			if (priorMarker->name.get() == name) {
				// If so, we will update the existing marker
				marker = priorMarker;
				break;
			}
		}

		// Otherwise make it
		if (!marker) {
			marker = make_shared<Marker>();
			marker->name.set(name);
		}
	}
	
	if (this->isBeingInspected()) {
		marker->position.set(this->cursorPosition);
	}
	else {
		marker->position.set(Scene::X()->getPanel()->getCamera().getCursorWorld());
	}
	this->add(marker);
	return marker;
}

//----------
void
Markers::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;
	AbstractCalibrationPointSet::populateInspector(args);
	inspector->addButton("Sort by name", [this]() {
		this->sortBy([](shared_ptr<Marker> a, shared_ptr<Marker> b) {
			return a->name.get() < b->name.get();
			});
		});
	inspector->addButton("Sort by date", [this]() {
		this->sortByDate();
		});
	inspector->addToggle(this->snapToVertex);
	inspector->addButton("Add...", [this]() {
		this->addMarker();
		}, ' ')->setHeight(100.0f);
}
