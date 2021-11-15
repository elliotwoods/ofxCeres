#include "pch_ofApp.h"
#include "Scene.h"
#include "Exception.h"
#include "DMX/FixtureFactory.h"

//----------
Scene::Scene()
{
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
		this->populateInspector(args);
	};

	this->panel->onDrawWorld += [this](ofCamera&) {
		this->drawWorld();
	};

	ofxCvGui::InspectController::X().onClear += [this](ofxCvGui::InspectArguments& args) {
		auto inspector = args.inspector;
		inspector->addFps();
		inspector->addButton("Save all", [this]() {
			this->save(Scene::getDefaultFilename());
			});
	};

	// OSC routes
	{
		this->addSubRouter("groupControl", this->groupControl);
		this->dynamicRoute([this](const OSC::Path& path, const ofxOscMessage& message) {
			if (path.size() > 2 && path[0] == "movingheads") {
				auto name = path[1];
				for (auto& it : this->movingHeads) {
					if (OSC::Path::stripName(it.first) == name) {
						auto innerPath = path.subPath().subPath();
						it.second->route(innerPath, message);
						return true;
					}
				}
			}
			return false;
		});
	}
}

//----------
shared_ptr<Scene>
Scene::X()
{
	static auto instance = shared_ptr<Scene>(new Scene());
	return instance;
}

//----------
void
Scene::update()
{
	for (const auto& movingHead : this->movingHeads) {
		movingHead.second->update();
	}

	this->groupControl->update();
	this->enttecUSBPro->update();
	this->renderDMX();
	this->mesh->update();
	this->groupSolve->update();

	// OSC
	{
		// clear OSC object if needs be
		if (this->oscReceiver) {
			if (!this->oscParameters.enabled) {
				this->oscReceiver.reset();
			}
			if (this->oscParameters.port.get() != this->oscReceiver->getPort()) {
				this->oscReceiver.reset();
			}
		}
		
		// create OSC object if needs be
		if(!this->oscReceiver && this->oscParameters.enabled) {
			ofxOscReceiverSettings settings;
			{
				settings.port = this->oscParameters.port.get();
			}
			this->oscReceiver = make_shared<ofxOscReceiver>();
			this->oscReceiver->setup(settings);
			if (!this->oscReceiver->isListening()) {
				ofLogError("OSC") << "Couldn't listen on port " << this->oscParameters.port.get();
				this->oscReceiver.reset();
				this->oscParameters.enabled.set(false);
			}
		}

		// route incoming OSC messages
		if(this->oscReceiver) {
			ofxOscMessage message;
			while (this->oscReceiver->getNextMessage(message)) {
				this->route(OSC::Path(message.getAddress()), message);
			}
		}
	}
}

//----------
void
Scene::drawWorld()
{
	// Draw the moving heads
	for (auto& movingHead : this->movingHeads) {
		movingHead.second->drawWorld();
		ofxCvGui::Utils::drawTextAnnotation(movingHead.first
			, movingHead.second->getModel()->getPosition());
	}

	this->markers->drawWorld();
	this->mesh->drawWorld();
	this->groupSolve->drawWorld();
	this->groupControl->drawWorld();
}

//----------
void
Scene::renderDMX()
{
	vector<DMX::Value> dmxValues(513, 0);
	for (auto& movingHead : this->movingHeads) {
		movingHead.second->getDMX(dmxValues);
	}
	this->enttecUSBPro->send(dmxValues);
}

//----------
void
Scene::serialize(nlohmann::json& json)
{
	// Save the static modules
	this->markers->notifySerialize(json["markers"]);
	this->mesh->notifySerialize(json["mesh"]);
	this->groupSolve->notifySerialize(json["groupSolve"]);
	this->enttecUSBPro->notifySerialize(json["enttecUSBPro"]);
	this->groupControl->notifySerialize(json["groupControl"]);
	Data::serialize(json["worldPanel"], this->panel->parameters);
	Data::serialize(json["OSC"], this->oscParameters);

	// Save the fixtures
	{
		auto& jsonMovingHeads = json["movingHeads"];

		for (auto it : this->movingHeads) {
			nlohmann::json jsonMovingHead;

			jsonMovingHead["typeName"] = it.second->getTypeName();
			jsonMovingHead["name"] = it.first;
			auto& content = jsonMovingHead["content"];
			it.second->notifySerialize(content);
			jsonMovingHeads.push_back(jsonMovingHead);
		}
	}
}

//----------
void
Scene::deserialize(const nlohmann::json& json)
{
	// Load the static modules
	if (json.contains("markers")) {
		this->markers->notifyDeserialize(json["markers"]);
	}
	if (json.contains("mesh")) {
		this->mesh->notifyDeserialize(json["mesh"]);
	}
	if (json.contains("groupSolve")) {
		this->groupSolve->notifyDeserialize(json["groupSolve"]);
	}
	if (json.contains("enttecUSBPro")) {
		this->enttecUSBPro->notifyDeserialize(json["enttecUSBPro"]);
	}
	if (json.contains("groupControl")) {
		this->groupControl->notifyDeserialize(json["groupControl"]);
	}
	if (json.contains("worldPanel")) {
		Data::deserialize(json["worldPanel"], this->panel->parameters);
	}
	if (json.contains("OSC")) {
		Data::deserialize(json["OSC"], this->oscParameters);
	}

	// Load the fixtures
	{
		// clear existing
		this->movingHeads.clear();

		const auto& jsonMovingHeads = json["movingHeads"];

		for (const auto& jsonMovingHead : jsonMovingHeads) {
			auto typeName = jsonMovingHead["typeName"].get<string>();
			auto name = jsonMovingHead["name"].get<string>();
			auto content = jsonMovingHead["content"];

			try {
				auto movingHead = Scene::makeMovingHead(typeName);
				movingHead->notifyDeserialize(content);
				this->movingHeads.emplace(name, movingHead);
			}
			CATCH_TO_ALERT;
		}
	}

	ofxCvGui::refreshInspector(this);
}

//--------------------------------------------------------------
void
Scene::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;

	inspector->addButton("Save as...", [this]() {
		auto result = ofSystemSaveDialog(Scene::getDefaultFilename(), "Save json...");
		if (result.bSuccess) {
			this->save(result.filePath);
		}
		});
	inspector->addButton("Load...", [this]() {
		auto result = ofSystemLoadDialog("Load json...");
		if (result.bSuccess) {
			this->load(result.filePath);
		}
		});
	inspector->addSubMenu(this->oscParameters);

	inspector->addSpacer();

	inspector->addTitle("Moving heads", ofxCvGui::Widgets::Title::Level::H2);
	{
		if (!this->movingHeads.empty()) {
			inspector->addTitle("Select a moving head:", ofxCvGui::Widgets::Title::Level::H3);
		}

		// Selection buttons for moving heads
		for (auto& it : this->movingHeads) {
			auto button = inspector->addSubMenu(it.first, it.second);
			button->setHeight(75.0f);
			auto deleteButton = make_shared<ofxCvGui::Widgets::Button>("", [it, this]() {
				this->deleteMovingHead(it.first);
				});
			{
				deleteButton->setDrawGlyph(u8"\uf2ed"); // trash can
				deleteButton->addToolTip("Delete");
			}
			auto renameButton = make_shared<ofxCvGui::Widgets::Button>("", [it, this]() {
				this->renameMovingHead(it.first);
				});
			{
				renameButton->setDrawGlyph(u8"\uf044"); // edit
				renameButton->addToolTip("Edit name");
			}
			button->onBoundsChange += [deleteButton, renameButton](ofxCvGui::BoundsChangeArguments& args) {
				ofRectangle bounds(
					args.localBounds.width - 45 - 30
					, 5
					, 40
					, args.localBounds.height - 10
				);
				deleteButton->setBounds(bounds);
				bounds.x -= bounds.width + 5;
				renameButton->setBounds(bounds);
			};
			button->addChild(deleteButton);
			button->addChild(renameButton);

			button->addToolTip(it.second->getTypeName() + " on Channel #" + ofToString(it.second->channelIndex.get()));
		}
	}
	inspector->addSubMenu("Add moving head", [this](ofxCvGui::InspectArguments& args) {
		auto hasName = [this](const string& name) {
			auto findName = this->movingHeads.find(name);
			return findName != this->movingHeads.end();
		};

		// Setup the name for new moving head
		{
			auto name = this->newMovingHead.name;
			if (name.get().empty() || hasName(name)) {
				for (int i = 1; ; i++) {
					auto name = ofToString(i);
					if (!hasName(name)) {
						this->newMovingHead.name = name;
						break;
					}
				}
			}
		}

		// Setup the DMX channel as first free channel
		{
			// This routine should render the dmx channels in-use
			vector<DMX::Value> dmxChannels(1);
			for (const auto& it : this->movingHeads) {
				it.second->getDMX(dmxChannels);
			}
			this->newMovingHead.dmxChannelIndex.set(dmxChannels.size());
		}

		auto inspector = args.inspector;
		inspector->addEditableValue(this->newMovingHead.name);
		inspector->addEditableValue(this->newMovingHead.dmxChannelIndex);
		auto typeSelector = inspector->addMultipleChoice("Type");
		{
			auto fixtureFactory = DMX::FixtureFactory::X();
			for (auto it : fixtureFactory) {
				typeSelector->addOption(it.first);
			}
		}
		inspector->addButton("Add", [this, hasName, typeSelector]() {
			try {
				auto name = this->newMovingHead.name.get();
				if (hasName(name)) {
					throw(Exception("Cannot add another moving head with same name '" + name + "'"));
				}

				auto movingHead = this->makeMovingHead(typeSelector->getSelection());
				movingHead->channelIndex.set(this->newMovingHead.dmxChannelIndex.get());
				this->movingHeads.emplace(name, movingHead);

				ofxCvGui::InspectController::X().back();
			}
			CATCH_TO_ALERT;
			}, OF_KEY_RETURN)->setHeight(100.0f);
		});

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

	inspector->addButton("Rotate scene", [this]() {
		this->rotateScene();
		});

	inspector->addSubMenu("Group Solve", this->groupSolve);
	inspector->addSubMenu("Group Control", this->groupControl);

	inspector->addSpacer();

	inspector->addSubMenu("Mesh", this->mesh);

	inspector->addSubMenu("World Panel", [this](ofxCvGui::InspectArguments& args) {
		auto inspector = args.inspector;
		inspector->addParameterGroup(this->panel->parameters);
		inspector->addButton("Fit world grid", [this]() {
			this->fitWorldGrid();
			})->addToolTip("Change the room min/max to match all data");
		inspector->addButton("Expand world grid", [this]() {
			{
				auto value = this->panel->parameters.grid.roomMin.get();
				value -= {1, 1, 1};
				this->panel->parameters.grid.roomMin.set(value);
			}
			{
				auto value = this->panel->parameters.grid.roomMax.get();
				value += {1, 1, 1};
				this->panel->parameters.grid.roomMax.set(value);
			}
			})->addToolTip("Add 1 unit in all directions");
		});
	

	inspector->addSubMenu("Enttec USB Pro", this->enttecUSBPro);
}

//--------------------------------------------------------------
void
Scene::load(const string& path)
{
	try {
		ofFile file;
		file.open(path);
		if (file.exists()) {
			// Load the json
			nlohmann::json json;
			file >> json;
			this->deserialize(json);

			this->panel->loadCamera(path + "-camera.txt");
		}
	}
	CATCH_TO_ALERT;
}

//--------------------------------------------------------------
void
Scene::save(string& path)
{
	// get json
	nlohmann::json json;
	this->serialize(json);

	// check extension
	{
		auto extension = ofToLower(ofFilePath::getFileExt(path));
		if (extension != "json") {
			path = path + ".json";
		}
	}

	// open file and put contents of json
	ofFile file;
	file.open(path, ofFile::Mode::WriteOnly);
	file << std::setw(4) << json;

	this->panel->saveCamera(path + "-camera.txt");
}

//----------
map<string, shared_ptr<DMX::MovingHead>>&
Scene::getMovingHeads()
{
	return this->movingHeads;
}

//----------
shared_ptr<Markers>
Scene::getMarkers()
{
	return this->markers;
}

//----------
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

//----------
void
Scene::renameMovingHead(const string& name)
{
	auto result = ofSystemTextBoxDialog("Rename moving head");
	if (!result.empty()) {
		auto findMovingHead = this->movingHeads.find(name);
		if (findMovingHead == this->movingHeads.end()) {
			throw(Exception("Moving head " + name + " not found. Cannot delete"));
		}
		auto movingHead = findMovingHead->second;
		this->movingHeads.erase(findMovingHead);
		this->movingHeads.emplace(result, movingHead);
	}

	ofxCvGui::InspectController::X().refresh(this);
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
				auto calibrationPoints = movingHead.second->getSolver()->getCalibrationPoints()->getAllCaptures();
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
		positions.push_back(movingHead.second->getModel()->getPosition());
	}
	if (this->mesh->isLoaded()) {
		positions.push_back(this->mesh->getMin());
		positions.push_back(this->mesh->getMax());
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
			movingHead.second->getModel()->applyRotation(rotation);
		}
	}
}

//----------
shared_ptr<ofxCvGui::Panels::WorldManaged>
Scene::getPanel()
{
	return this->panel;
}

//----------
shared_ptr<DMX::MovingHead>
Scene::makeMovingHead(const string& typeName)
{
	auto findFactory = DMX::FixtureFactory::X().find(typeName);
	if (findFactory == DMX::FixtureFactory::X().end()) {
		throw(Exception("Factory '" + typeName + "' not found"));
	}

	auto fixture = findFactory->second->makeUntyped();
	auto movingHead = dynamic_pointer_cast<DMX::MovingHead>(fixture);
	if (!movingHead) {
		throw(Exception("We can only handle moving heads right now"));
	}

	return movingHead;
}

//----------
string
Scene::getDefaultFilename()
{
	return "MovingHeads.json";
}