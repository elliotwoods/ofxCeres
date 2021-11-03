#include "pch_ofApp.h"
#include "GroupSolve.h"
#include "Scene.h"

//----------
GroupSolve::GroupSolve(Scene& scene)
	: scene(scene)
{
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
		this->populateInspector(args);
	};
	this->onSerialize += [this](nlohmann::json& json) {
		this->serialize(json);
	};
	this->onDeserialize += [this](const nlohmann::json& json) {
		this->deserialize(json);
	};
}

//----------
string
GroupSolve::getTypeName() const
{
	return "GroupSolve";
}

//----------
void
GroupSolve::serialize(nlohmann::json&)
{

}

//----------
void
GroupSolve::deserialize(const nlohmann::json&)
{

}

//----------
void
GroupSolve::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;

	inspector->addButton("Select only multi-use markers", [this]() {
		this->selectOnlyMultiUseMarkers();
		});

	inspector->addButton("Solve", [this]() {
		try {
			this->solve();
		}
		CATCH_TO_ALERT;
		});

}

//----------
void
GroupSolve::selectOnlyMultiUseMarkers()
{
	auto markers = this->scene.getMarkers();
	auto movingHeads = this->scene.getMovingHeads();

	auto selectedMarkers = markers->getSelection();
	for (auto marker : selectedMarkers) {
		auto markerName = marker->name.get();

		bool seenInOneMovingHead = false;
		bool seenInMoreThanOneMovingHead = false;

		for (auto movingHead : movingHeads) {
			auto calibrationPoints = movingHead.second->getCalibrationPoints()->getSelection();
			bool seenInThisMovingHead = false;
			for (auto calibrationPoint : calibrationPoints) {
				if (calibrationPoint->marker.get() == markerName) {
					seenInThisMovingHead = true;
					break;
				}
			}
			if (seenInThisMovingHead) {
				if (seenInOneMovingHead) {
					seenInMoreThanOneMovingHead = true;
					break;
				}
				else {
					seenInOneMovingHead = true;
				}
			}
		}

		if (!seenInMoreThanOneMovingHead) {
			marker->setSelected(false);
		}
	}
}

//----------
void
GroupSolve::solve()
{

}