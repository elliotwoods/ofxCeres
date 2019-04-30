#include "pch_ofApp.h"

#include "AbstractCalibrationPointSet.h"
#include "ofxCvGui.h"

using namespace ofxCvGui;

namespace Data {
#pragma mark AbstractCalibrationPointSet
	//----------
	AbstractCalibrationPointSet::AbstractCalibrationPointSet() {
		RULR_SERIALIZE_LISTENERS;

		this->listView = Panels::makeWidgets();
		this->listView->setHeight(400.0f);
		this->listView->onUpdate += [this](ofxCvGui::UpdateArguments &) {
			if (this->viewDirty) {
				this->listView->clear();
				for (auto capture : this->captures) {
					this->listView->add(capture->getGuiElement());
				}
				this->viewDirty = false;
			}
		};
		this->listView->onDraw += [this](DrawArguments & args) {
			ofPushStyle();
			{
				ofNoFill();
				ofSetLineWidth(1.0f);
				ofDrawRectangle(args.localBounds);
			}
			ofPopStyle();
		};
		this->listView->setScissorEnabled(true);
	}

	//----------
	AbstractCalibrationPointSet::~AbstractCalibrationPointSet() {
		this->clear();
	}

	//----------
	void AbstractCalibrationPointSet::add(shared_ptr<AbstractCalibrationPoint> capture) {
		if (find(this->captures.begin(), this->captures.end(), capture) != this->captures.end()) {
			return;
		}

		auto captureWeak = weak_ptr<AbstractCalibrationPoint>(capture);
		capture->onDeletePressed += [captureWeak, this]() {
			this->listView->clear();
			this->viewDirty = true;
			auto capture = captureWeak.lock();
			if (capture) {
				this->remove(capture);
			}
		};

		capture->onChange += [this]() {
			this->onChange.notifyListeners();
		};

		capture->onSelectionChanged += [captureWeak, this](bool selection) {
			if (!this->getIsMultipleSelectionAllowed() && selection) {
				auto selectionSet = this->getSelectionUntyped();
				auto selectedCapture = captureWeak.lock();
				for (auto otherCapture : selectionSet) {
					if (otherCapture != selectedCapture && otherCapture->isSelected()) {
						otherCapture->setSelected(false);
					}
				}
			}
			this->onSelectionChanged.notifyListeners();
		};

		this->onChange.notifyListeners();
		this->onSelectionChanged.notifyListeners();

		this->captures.push_back(capture);
		this->viewDirty = true;
	}

	//----------
	void AbstractCalibrationPointSet::remove(shared_ptr<AbstractCalibrationPoint> capture) {
		auto findCapture = find(this->captures.begin(), this->captures.end(), capture);
		if (findCapture != this->captures.end()) {
			if (capture) {
				capture->setSelected(false); // to notify upwards (onChange, onSelectionChange)
				capture->onDeletePressed.removeListeners(this);
				capture->onSelectionChanged.removeListeners(this);
			}
			this->captures.erase(findCapture);
			this->viewDirty = true;
		}
	}

	//----------
	void AbstractCalibrationPointSet::clear() {
		this->listView->clear();
		while (!this->captures.empty()) {
			this->remove(*this->captures.begin());
		}
		this->viewDirty = true;
	}

	//----------
	void AbstractCalibrationPointSet::select(shared_ptr<AbstractCalibrationPoint> capture) {
		capture->setSelected(true);
	}

	//----------
	void AbstractCalibrationPointSet::selectAll() {
		for (auto capture : this->captures) {
			capture->setSelected(true);
		}
	}

	//----------
	void AbstractCalibrationPointSet::selectNone() {
		for (auto capture : this->captures) {
			capture->setSelected(false);
		}
	}

	//----------
	void AbstractCalibrationPointSet::populateWidgets(shared_ptr<ofxCvGui::Panels::Widgets> widgetsPanel) {
		widgetsPanel->add(this->listView);

		widgetsPanel->addLiveValue<int>("Count", [this]() {
			return this->captures.size();
		});

		widgetsPanel->addButton("Clear", [this]() {
			this->clear();
		});

		{
			auto selectionSelector = widgetsPanel->addMultipleChoice("Selection", { "All", "None" });
			selectionSelector->setAllowNullSelection(true);
			selectionSelector->onValueChange += [this](int selectionIndex) {
				switch (selectionIndex) {
				case 0:
					this->selectAll();
					break;
				case 1:
					this->selectNone();
					break;
				default:
					break;
				}
			};
			auto selectionSelectorWeak = weak_ptr<Widgets::MultipleChoice>(selectionSelector);
			selectionSelector->onUpdate += [this, selectionSelectorWeak](UpdateArguments &) {
				auto selectionSelector = selectionSelectorWeak.lock();
				if (selectionSelector) {
					if (this->captures.empty()) {
						selectionSelector->setSelection(-1);
					}
					bool allSelected = true;
					bool noneSelected = true;
					for (auto capture : this->captures) {
						if (capture->isSelected()) {
							noneSelected = false;
						}
						else {
							allSelected = false;
						}
					}
					if (allSelected) {
						selectionSelector->setSelection(0);
					}
					else if (noneSelected) {
						selectionSelector->setSelection(1);
					}
					else {
						selectionSelector->setSelection(-1);
					}
				}
			};
		}

		widgetsPanel->addSpacer();
	}

	//----------
	void AbstractCalibrationPointSet::serialize(nlohmann::json & json) {
		nlohmann::json jsonCaptures;
		int index = 0;
		for (auto capture : this->captures) {
			nlohmann::json captureJson;
			capture->serialize(captureJson);
			jsonCaptures.emplace_back(move(captureJson));
		}
		json["captures"] = jsonCaptures;
	}

	//----------
	void AbstractCalibrationPointSet::deserialize(const nlohmann::json & json) {
		this->captures.clear();
		if (json.count("captures") != 0) {
			const auto & jsonCaptures = json["captures"];
			for (const auto & jsonCapture : jsonCaptures) {
				try {
					auto capture = this->makeEmpty();
					capture->deserialize(jsonCapture);
					this->add(capture); //ensure event listeners are attached
				}
				catch (const std::exception & e) {
					ofLogError("AbstractCalibrationPointSet::deserialize") << e.what();
				}
			}
		}
	}

	//----------
	vector<shared_ptr<AbstractCalibrationPoint>> AbstractCalibrationPointSet::getSelectionUntyped() const {
		auto selection = this->captures;
		for (auto it = selection.begin(); it != selection.end(); ) {
			if (!(*it)->isSelected()) {
				it = selection.erase(it);
			}
			else {
				it++;
			}
		}
		return selection;
	}

	//----------
	vector<shared_ptr<AbstractCalibrationPoint>> AbstractCalibrationPointSet::getAllCapturesUntyped() const {
		return this->captures;
	}

	//----------
	size_t AbstractCalibrationPointSet::size() const {
		return this->captures.size();
	}
}
