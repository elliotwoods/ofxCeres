#include "pch_ofApp.h"
#include "Solver.h"
#include "DMX/MovingHead.h"
#include "Scene.h"
#include "Widgets/PanTiltTrackpad.h"

namespace Calibration {
	//----------
	Solver::Solver(DMX::MovingHead& movingHead)
		: movingHead(movingHead)
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;

		// listen for mouse events on view
		{
			auto worldPanel = Scene::X()->getPanel();
			worldPanel->onMouse.addListener([this](ofxCvGui::MouseArguments& args) {
				if (this->isBeingInspected()) {
					auto scene = Scene::X();
					auto cursorWorld = scene->getPanel()->getCamera().getCursorWorld();
					this->markerClosestToCursor = scene->getMarkers()->getMarkerClosestTo(cursorWorld);
				}
				}, this);
		}

		// store here because otherwise we might accidentally call Scene::X() from Solver::~Solver() whilst Scene is being destroyed
		this->scene = Scene::X();
	}

	//----------
	Solver::~Solver()
	{
		// remove listeners for mouse events on view
		{
			this->scene->getPanel()->onMouse.removeListeners(this);
		}
	}

	//----------
	string
	Solver::getTypeName() const
	{
		return "Calibration::Solver";
	}

	//----------
	void
	Solver::drawWorld()
	{
		auto selectedMarker = this->markerClosestToCursor.lock();
		if (selectedMarker) {
			ofxCvGui::Utils::drawTextAnnotation(ofToString(selectedMarker->position.get(), 3)
				, selectedMarker->position.get()
				, selectedMarker->color);
		}
	}

	//---------
	void
	Solver::serialize(nlohmann::json& json)
	{
		Data::serialize(json, this->solverSettings);

		{
			ofParameter<string> solveTypeName{ "Solve type", this->parameters.solveType.get().toString() };
			json["parameters"] << solveTypeName;
		}

		this->calibrationPoints->serialize(json);
	}

	//---------
	void
	Solver::deserialize(const nlohmann::json& json)
	{
		Data::deserialize(json, this->solverSettings);

		if (json.contains("parameters")) {
			ofParameter<string> solveTypeName{ "Solve type", "" };
			json["parameters"] >> solveTypeName;

			// set the parameter
			{
				auto solveType = this->parameters.solveType.get();
				solveType.fromString(solveTypeName);
				this->parameters.solveType.set(solveType);
			}
		}

		this->calibrationPoints->deserialize(json);
	}

	//---------
	void
	Solver::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;

		inspector->addTitle("Calibration points", ofxCvGui::Widgets::Title::H2);
		this->calibrationPoints->populateInspector(args);

		{
			auto trackpad = make_shared<Widgets::PanTiltTrackpad>(this->movingHead.parameters.pan, this->movingHead.parameters.tilt);
			auto trackpadWeak = weak_ptr<Widgets::PanTiltTrackpad>(trackpad);
			trackpad->onDraw += [this, trackpadWeak](ofxCvGui::DrawArguments& args) {
				auto trackpadWidget = trackpadWeak.lock();

				// draw the existing selected data points onto the trackpad
				ofMesh pointsPreview;
				pointsPreview.setMode(ofPrimitiveMode::OF_PRIMITIVE_POINTS);

				auto calibrationPoints = this->calibrationPoints->getSelection();
				for (auto calibrationPoint : calibrationPoints) {
					pointsPreview.addColor(calibrationPoint->color.get());
					pointsPreview.addVertex(glm::vec3(trackpadWidget->toXY(calibrationPoint->panTiltSignal.get()), 0.0f));
				}
				pointsPreview.draw();
			};
			inspector->add(trackpad);
		}

		inspector->addButton("Add", [this]() {
			try {
				this->addCalibrationPoint();
			}
			CATCH_TO_ALERT;
			}, ' ');
		inspector->addTitle("Solve", ofxCvGui::Widgets::Title::H2);
		{
			auto widget = inspector->addMultipleChoice(this->parameters.solveType.getName());
			widget->entangleManagedEnum(this->parameters.solveType);
		}

		inspector->addSubMenu(this->solverSettings);
	}

	//---------
	void
	Solver::addCalibrationPoint()
	{
		auto marker = this->markerClosestToCursor.lock();
		if (!marker) {
			throw(Exception("No marker selected"));
		}

		auto calibrationPoint = make_shared<DataPoint>();
		{
			calibrationPoint->panTiltSignal.set(this->movingHead.getCurrentPanTilt());
			calibrationPoint->marker = marker->name.get();
		}
		this->calibrationPoints->add(calibrationPoint);

		ofxCvGui::refreshInspector(this);
	}

	//---------
	void
	Solver::solve()
	{

	}

	//---------
	shared_ptr<Data::CalibrationPointSet<DataPoint>>
	Solver::getCalibrationPoints()
	{
		return this->calibrationPoints;
	}
}