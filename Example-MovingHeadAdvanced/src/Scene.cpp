#include "pch_ofApp.h"
#include "Scene.h"
#include "Exception.h"

//----------
Scene::Scene() {
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
		this->populateInspector(args);
	};
}

//----------
void Scene::update() {
	for (const auto& movingHead : this->movingHeads) {
		movingHead.second->update();
	}
}

//----------
void Scene::drawWorld() {
	auto sceneIsSelected = this->isBeingInspected();

	// Draw the moving heads
	for (auto& movingHead : this->movingHeads) {
		bool isSelected = movingHead.first == this->selection;
		if (isSelected) {
			movingHead.second->drawWorld(true);
		}
		else if (this->drawOtherFixtures) {
			movingHead.second->drawWorld(false);
		}

		if (sceneIsSelected) {
			ofxCvGui::Utils::drawTextAnnotation(movingHead.first, movingHead.second->getPosition());
		}
	}
}

//----------
void Scene::renderDMX(vector<uint8_t>& dmxValues) {
	for (auto& movingHead : this->movingHeads) {
		movingHead.second->renderDMX(dmxValues);
	}
}

//--------------------------------------------------------------
void Scene::populateInspector(ofxCvGui::InspectArguments& args) {
	auto inspector = args.inspector;

	inspector->addFps();
	inspector->addButton("Save all", [this]() {
		this->save();
		});

	inspector->addSpacer();

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

	inspector->addTitle("Select a moving head:", ofxCvGui::Widgets::Title::Level::H2);

	// Selection buttons for moving heads
	for (auto& it : this->movingHeads) {
		auto button = make_shared<ofxCvGui::Widgets::Button>(it.first, [it, this]() {
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

//--------------------------------------------------------------
void Scene::load() {
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
void Scene::save() {
	for (const auto& it : this->movingHeads) {
		nlohmann::json json;
		it.second->serialize(json);

		ofFile file;
		file.open(it.first + ".json", ofFile::Mode::WriteOnly);
		file << std::setw(4) << json;
	}
}

//--------------------------------------------------------------
map<string, shared_ptr<MovingHead>>& Scene::getMovingHeads() {
	return this->movingHeads;
}

//--------------------------------------------------------------
shared_ptr<MovingHead> Scene::addMovingHead(const string& name) {
	//Check if same name already exists
	for (const auto& movingHead : this->movingHeads) {
		if (movingHead.first == name) {
			throw(Exception(name + " already exists. Cannot add 2 moving heads with same name"));
		}
	}

	auto movingHead = make_shared<MovingHead>();

	this->addMovingHead(name, movingHead);
	
	return movingHead;
}

//--------------------------------------------------------------
void Scene::addMovingHead(const string& name, shared_ptr<MovingHead> movingHead) {
	this->movingHeads.emplace(name, movingHead);
	ofxCvGui::InspectController::X().refresh(this);
}

//--------------------------------------------------------------
void Scene::deleteMovingHead(const string& name) {
	auto findMovingHead = this->movingHeads.find(name);
	if (findMovingHead == this->movingHeads.end()) {
		throw(Exception("Moving head " + name + " not found. Cannot delete"));
	}
	this->movingHeads.erase(findMovingHead);

	ofxCvGui::InspectController::X().refresh(this);
}

//--------------------------------------------------------------
void Scene::importMovingHead() {
	auto result = ofSystemLoadDialog("Load moving head...");
	if (!result.bSuccess) {
		return;
	}

	ofFile file;
	file.open(result.filePath);
	if (!file.is_open()) {
		throw(Exception("Couldn't open file " + result.filePath));
	}

	auto movingHead = make_shared<MovingHead>();
	auto name = ofFilePath::getBaseName(result.filePath);

	nlohmann::json json;
	file >> json;
	movingHead->deserialize(json);

	this->addMovingHead(name, movingHead);
}