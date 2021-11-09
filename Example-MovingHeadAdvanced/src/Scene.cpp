#include "pch_ofApp.h"
#include "Scene.h"
#include "Exception.h"

//----------
Scene::Scene()
{
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
		this->populateInspector(args);
	};

	this->panel = ofxCvGui::Panels::makeWorldManaged();
	this->panel->onDrawWorld += [this](ofCamera&) {
		this->drawWorld();
	};
}

//----------
void
Scene::update()
{
	for (const auto& movingHead : this->movingHeads) {
		movingHead.second->update();
	}

	this->testMovingHead->update();
	this->enttecUSBPro->update();

	this->renderDMX();
}

//----------
void
Scene::drawWorld()
{
	auto sceneIsSelected = this->isBeingInspected();

	// Draw the moving heads
	auto groupSolveSelected = this->groupSolve->isBeingInspected();
	for (auto& movingHead : this->movingHeads) {
		bool isSelected = movingHead.first == this->selection;
		if (isSelected || groupSolveSelected) {
			movingHead.second->drawWorld(true);
		}
		else if (this->drawOtherFixtures) {
			movingHead.second->drawWorld(false);
		}

		ofxCvGui::Utils::drawTextAnnotation(movingHead.first, movingHead.second->getPosition());
	}

	this->markers->drawWorld();

	// Draw the mesh
	this->mesh->drawWorld();
}

//----------
void
Scene::renderDMX()
{
	vector<DMX::Value> dmxValues(513, 0);
	for (auto& movingHead : this->movingHeads) {
		movingHead.second->renderDMX(dmxValues);
	}
	this->testMovingHead->getDMX(dmxValues);
	this->enttecUSBPro->send(dmxValues);
}

//--------------------------------------------------------------
void
Scene::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;

	inspector->addFps();
	inspector->addButton("Save all", [this]() {
		this->save();
		});

	inspector->addSpacer();

	inspector->addTitle("Moving heads", ofxCvGui::Widgets::Title::Level::H2);
	{
		inspector->addButton("Add moving head...", [this]() {
			auto response = ofSystemTextBoxDialog("Moving head name");
			if (!response.empty()) {
				try {
					this->addMovingHead(response);
				}
				CATCH_TO_ALERT;
			}
			});
		{
			auto button = make_shared<ofxCvGui::Widgets::Button>("Import moving head...", [this]() {
				try {
					this->importMovingHead();
				}
				CATCH_TO_ALERT;
				});
			button->addToolTip("Import from older version of software");
			inspector->add(button);
		}

		if (!this->movingHeads.empty()) {
			inspector->addTitle("Select a moving head:", ofxCvGui::Widgets::Title::Level::H3);
		}

		// Selection buttons for moving heads
		for (auto& it : this->movingHeads) {
			auto button = make_shared<ofxCvGui::Widgets::Button>(it.first + " >>", [it, this]() {
				this->selection = it.first;
				ofxCvGui::inspect(this->movingHeads[this->selection]);
				});

			auto deleteButton = make_shared<ofxCvGui::Widgets::Button>("X", [it, this]() {
				this->deleteMovingHead(it.first);
				});
			button->onBoundsChange += [deleteButton](ofxCvGui::BoundsChangeArguments& args) {
				ofRectangle bounds(
					args.localBounds.width - 45
					, 5
					, 40
					, args.localBounds.height - 10
				);
				deleteButton->setBounds(bounds);
			};
			button->addChild(deleteButton);
			inspector->add(button);
		}
	}

	inspector->addSpacer();

	inspector->addTitle("Markers", ofxCvGui::Widgets::Title::Level::H2);
	{
		inspector->addLiveValue<size_t>("Active markers", [this]() {
			return this->markers->getSelection().size();
			});
		inspector->addSubMenu("Markers", this->markers);
		inspector->addButton("Merge markers", [this]() {
			try {
				this->mergeMarkers();
			}
			CATCH_TO_ALERT;
			})->addToolTip("Merge markers that share the same position");
	}

	inspector->addSpacer();

	inspector->addSubMenu("Group Solve", this->groupSolve);

	inspector->addSpacer();

	inspector->addSubMenu("Mesh", this->mesh);

	inspector->addButton("Fit world grid", [this]() {
		this->fitWorldGrid();
		})->addToolTip("Change the room min/max to match all data");
	inspector->addButton("Rotate scene", [this]() {
		this->rotateScene();
		})->addToolTip("Rotate around +x axis by 90 degrees");

	inspector->addSubMenu("Sharpy", this->testMovingHead);
	inspector->addSubMenu("Enttec USB Pro", this->enttecUSBPro);
}

//--------------------------------------------------------------
void
Scene::load()
{
	for (const auto& it : this->movingHeads) {
		ofFile file;
		file.open(it.first + ".json");
		if (file.exists()) {
			nlohmann::json json;
			file >> json;
			it.second->deserialize(json);
		}
	}
}

//--------------------------------------------------------------
void
Scene::save()
{
	for (const auto& it : this->movingHeads) {
		nlohmann::json json;
		it.second->serialize(json);

		ofFile file;
		file.open(it.first + ".json", ofFile::Mode::WriteOnly);
		file << std::setw(4) << json;
	}
}

//--------------------------------------------------------------
map<string, shared_ptr<MovingHead>>&
Scene::getMovingHeads()
{
	return this->movingHeads;
}

//--------------------------------------------------------------
shared_ptr<Markers>
Scene::getMarkers()
{
	return this->markers;
}

//--------------------------------------------------------------
shared_ptr<MovingHead>
Scene::addMovingHead(const string& name)
{
	//Check if same name already exists
	for (const auto& movingHead : this->movingHeads) {
		if (movingHead.first == name) {
			throw(Exception(name + " already exists. Cannot add 2 moving heads with same name"));
		}
	}

	auto movingHead = make_shared<MovingHead>(this->markers);

	this->addMovingHead(name, movingHead);
	
	return movingHead;
}

//--------------------------------------------------------------
void
Scene::addMovingHead(const string& name, shared_ptr<MovingHead> movingHead)
{
	this->movingHeads.emplace(name, movingHead);
	ofxCvGui::InspectController::X().refresh(this);
}

//--------------------------------------------------------------
void
Scene::deleteMovingHead(const string& name)
{
	auto findMovingHead = this->movingHeads.find(name);
	if (findMovingHead == this->movingHeads.end()) {
		throw(Exception("Moving head " + name + " not found. Cannot delete"));
	}
	this->movingHeads.erase(findMovingHead);

	ofxCvGui::InspectController::X().refresh(this);
}

//--------------------------------------------------------------
void
Scene::importMovingHead()
{
	auto result = ofSystemLoadDialog("Load moving head...");
	if (!result.bSuccess) {
		return;
	}

	ofFile file;
	file.open(result.filePath);
	if (!file.is_open()) {
		throw(Exception("Couldn't open file " + result.filePath));
	}

	auto movingHead = make_shared<MovingHead>(this->markers);
	auto name = ofFilePath::getBaseName(result.filePath);

	nlohmann::json json;
	file >> json;
	movingHead->deserialize(json, true);

	// Bring all the calibration points into the markers
	if (json.contains("calibrationPoints")) {
		const auto& jsonCalibrationPoints = json["calibrationPoints"];
		if (jsonCalibrationPoints.contains("captures")) {
			const auto& jsonCaptures = jsonCalibrationPoints["captures"];
			for (const auto& jsonCapture : jsonCaptures) {
				// Use standard deserialisation (i.e. dummy parameters) to get the data out
				ofParameter<glm::vec3> position{ "Target point", {0, 0, 0} };
				ofParameter<string> name{ "Name", "" };
				jsonCapture >> position;
				jsonCapture >> name;

				auto marker = this->markers->addNewMarker(name, position, true);
				this->markers->add(marker);
			}
		}
	}

	this->addMovingHead(name, movingHead);

	// Fit the world grid to accomodate the new data
	this->fitWorldGrid();
}

//----------
void
Scene::mergeMarkers()
{
	// Multiple markers might have the same position (e.g. if imported from prior version of software)
	// In this case, merge them into a single point and update the calibration points inside the moving heads

	auto allMarkers = this->markers->getAllCaptures();

	// Sort the markers by name
	map<string, shared_ptr<Marker>> sortedMarkers;
	for (auto marker : allMarkers) {
		sortedMarkers.emplace(marker->name.get(), marker);
	}

	// Count it as matching if distance is less than this
	// We can expect some numbers to be not exactly the same due to serialisation and other rounding errors
	auto epsilon = 1e-5;

	for (auto it = sortedMarkers.begin(); it != sortedMarkers.end(); it++) {
		auto mergedMarker = it->second;

		// Start looking at next and beyond
		auto it2 = it;
		it2++;
		
		for (; it2 != sortedMarkers.end(); ) {
			auto markerToMerge = it2->second;
			if (glm::distance(markerToMerge->position.get(), mergedMarker->position.get()) > epsilon) {
				// just continue if position doesn't match
				it2++;
				continue;
			}

			// Now we have a marker that we want to merge
			
			// Edit all moving heads calibration points that reference this marker to use the merged marker instead
			for (auto& movingHead : this->movingHeads) {
				auto calibrationPoints = movingHead.second->getCalibrationPoints()->getAllCaptures();
				for (auto& calibrationPoint : calibrationPoints) {
					if (calibrationPoint->marker.get() == markerToMerge->name.get()) {
						// We have a match for the marker that is being deleted
						calibrationPoint->marker.set(mergedMarker->name.get());
					}
				}
			}

			// Delete this marker from set
			this->markers->remove(markerToMerge);

			// Delete this marker from sorted list before continuing
			it2 = sortedMarkers.erase(it2);
		}
	}
}

//----------
void
Scene::fitWorldGrid()
{
	// First gather all positions
	vector<glm::vec3> positions;
	auto markers = this->markers->getSelection();
	for (auto marker : markers) {
		positions.push_back(marker->position);
	}
	for (const auto& movingHead : this->movingHeads) {
		positions.push_back(movingHead.second->getPosition());
	}

	// Now expand our min and max
	glm::vec3 min{ 0,0,0 };
	glm::vec3 max{ 0,0,0 };

	for (const auto& position : positions) {
		for (size_t i = 0; i < 3; i++) {
			while (position[i] < min[i]) {
				min[i] -= 1.0f;
			}
			while (position[i] > max[i]) {
				max[i] += 1.0f;
			}
		}
	}

	// And put them back into the panel
	this->panel->parameters.grid.roomMin.set(min);
	this->panel->parameters.grid.roomMax.set(max);
}

//----------
void
Scene::rotateScene()
{
	auto rotation = glm::angleAxis((float) PI / 2.0f, glm::vec3( 1, 0, 0 ));

	// Rotate markers
	{
		auto markers = this->markers->getAllCaptures();
		for (auto marker : markers) {
			marker->position.set(rotation * marker->position.get());
		}
	}

	// Rotate moving heads
	{
		for (const auto& movingHead : this->movingHeads) {
			movingHead.second->applyRotation(rotation);
		}
	}
}

//----------
shared_ptr<ofxCvGui::Panels::WorldManaged>
Scene::getPanel()
{
	return this->panel;
}