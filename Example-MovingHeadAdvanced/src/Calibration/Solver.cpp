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

		if (this->isBeingInspected()) {
			this->drawRaysAndResiduals();
		}
	}

	//----------
	void
	Solver::drawRaysAndResiduals()
	{
		auto calibrationPoints = this->calibrationPoints->getSelection();
		auto model = this->movingHead.getModel();
		ofPushStyle();
		{
			for (const auto& calibrationPoint : calibrationPoints) {
				auto marker = Scene::X()->getMarkers()->getMarkerByName(calibrationPoint->marker);
				if (!marker) {
					// Don't draw the rays for missing markers
					continue;
				}

				ofSetColor(calibrationPoint->color);

				auto idealPanTilt = model->panTiltSignalToIdeal(calibrationPoint->panTiltSignal.get());
				auto transmissionObject = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt(idealPanTilt);

				// Set length to distance of marker to moving head
				auto distance = glm::distance(marker->position.get(), model->getPosition());
				transmissionObject *= distance;

				auto transmissionWorld = ofxCeres::VectorMath::applyTransform(model->getTransform(), transmissionObject);

				ofSetLineWidth(2.0f);
				ofDrawLine(model->getPosition(), transmissionWorld);

				if (marker) {
					ofSetLineWidth(1.0f);
					ofSetColor(100);
					ofDrawLine(transmissionWorld, marker->position);
				}

			}
		}
		ofPopStyle();
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
		{
			auto dataPoints = this->calibrationPoints->getAllCaptures();
			for (auto dataPoint : dataPoints) {
				this->prepareDataPoint(dataPoint);
			}
		}
	}

	//---------
	void
	Solver::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;

		inspector->addTitle("Calibration points", ofxCvGui::Widgets::Title::H2);
		this->calibrationPoints->populateInspector(args);
		inspector->addButton("Sort by marker name", [this]() {
			this->calibrationPoints->sortBy([](shared_ptr<DataPoint> dataPointA, shared_ptr<DataPoint> dataPointB) {
				return dataPointA->marker.get() < dataPointB->marker.get();
				});
			});
		inspector->addButton("Sort by date", [this]() {
			this->calibrationPoints->sortByDate();
			});
		{
			auto trackpad = make_shared<Widgets::PanTiltTrackpad>(this->movingHead.parameters.pan, this->movingHead.parameters.tilt);
			auto trackpadWeak = weak_ptr<Widgets::PanTiltTrackpad>(trackpad);
			trackpad->onDraw += [this, trackpadWeak](ofxCvGui::DrawArguments& args) {
				auto trackpadWidget = trackpadWeak.lock();

				// draw the existing selected data points onto the trackpad
				ofMesh pointsPreview;
				pointsPreview.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINES);

				auto calibrationPoints = this->calibrationPoints->getSelection();
				for (auto calibrationPoint : calibrationPoints) {
					pointsPreview.addColor(calibrationPoint->color.get());
					pointsPreview.addColor(calibrationPoint->color.get());
					pointsPreview.addColor(calibrationPoint->color.get());
					pointsPreview.addColor(calibrationPoint->color.get());
					auto crossCenter = glm::vec3(trackpadWidget->toXY(calibrationPoint->panTiltSignal.get()), 0.0f);
					pointsPreview.addVertex(crossCenter - glm::vec3(0, 5, 0));
					pointsPreview.addVertex(crossCenter + glm::vec3(0, 5, 0));
					pointsPreview.addVertex(crossCenter - glm::vec3(5, 0, 0));
					pointsPreview.addVertex(crossCenter + glm::vec3(5, 0, 0));
				}

				for (size_t i = 0; i < pointsPreview.getNumVertices(); i++) {
					pointsPreview.addIndex(i);
				}
				pointsPreview.draw();
			};
			inspector->add(trackpad);
		}

		inspector->addSlider(this->movingHead.parameters.focus);


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
			inspector->addSubMenu(this->solverSettings);
			inspector->addButton("Solve", [this]() {
				try {
					this->solve();
				}
				CATCH_TO_ALERT
				}, OF_KEY_RETURN)->setHeight(100.0f);
		}

		inspector->addTitle("Treat data", ofxCvGui::Widgets::Title::H2);
		{
			inspector->addButton("Invert pan", [this]() {
				auto dataPoints = this->calibrationPoints->getSelection();
				for (auto dataPoint : dataPoints) {
					auto panTilt = dataPoint->panTiltSignal.get();
					panTilt.x = -panTilt.x;
					dataPoint->panTiltSignal.set(panTilt);
				}
				});
		}
		inspector->addTitle("Filter data", ofxCvGui::Widgets::Title::H2);
		{
			inspector->addButton("Positive tilt", [this]() {
				auto dataPoints = this->calibrationPoints->getSelection();
				for (auto dataPoint : dataPoints) {
					if (dataPoint->panTiltSignal.get().y < 0) {
						dataPoint->setSelected(false);
					}
				}
				});
			inspector->addButton("Negative tilt", [this]() {
				auto dataPoints = this->calibrationPoints->getSelection();
				for (auto dataPoint : dataPoints) {
					if (dataPoint->panTiltSignal.get().y > 0) {
						dataPoint->setSelected(false);
					}
				}
				});
		}

	}

	//---------
	void
	Solver::addCalibrationPoint()
	{
		auto marker = this->markerClosestToCursor.lock();
		if (!marker) {
			throw(Exception("No marker selected"));
		}

		auto dataPoint = make_shared<DataPoint>();
		{
			dataPoint->panTiltSignal.set(this->movingHead.getCurrentPanTilt());
			dataPoint->marker = marker->name.get();
		}
		this->prepareDataPoint(dataPoint);
		this->calibrationPoints->add(dataPoint);

		ofxCvGui::refreshInspector(this);
	}

	//---------
	void
	Solver::solve()
	{
		switch (this->parameters.solveType.get().get()) {
		case SolveType::Basic:
			this->solveBasic();
			break;
		case SolveType::Distorted:
			this->solveDistorted();
			break;
		case SolveType::Group:
			this->solveGroup();
			break;
		default:
			break;
		}
	}

	//----------
	void
	Solver::solveBasic()
	{
		vector<glm::vec3> targetPoints;
		vector<glm::vec2> panTiltSignal;
		this->getCalibrationData(targetPoints, panTiltSignal);

		auto priorSolution = ofxCeres::Models::MovingHead::Solution{
			this->movingHead.getModel()->parameters.translation.get()
				, this->movingHead.getModel()->parameters.rotationVector.get()
		};

		auto result = ofxCeres::Models::MovingHead::solve(targetPoints
			, panTiltSignal
			, priorSolution
			, this->solverSettings.getSolverSettings());

		this->movingHead.getModel()->parameters.translation = result.solution.translation;
		this->movingHead.getModel()->parameters.rotationVector = result.solution.rotationVector;
		this->movingHead.getModel()->parameters.panDistortion.set({
			0
			, 1
			, 0
			});
		this->movingHead.getModel()->parameters.tiltDistortion.set({
			0
			, 1
			, 0
			});

		// Convert residual into degrees
		this->movingHead.getModel()->parameters.residual.set(sqrt(result.residual / targetPoints.size()) * RAD_TO_DEG);
	}

	//----------
	void
	Solver::solveDistorted()
	{
		vector<glm::vec3> targetPoints;
		vector<glm::vec2> panTiltSignal;
		this->getCalibrationData(targetPoints, panTiltSignal);

		// Initialise the solution based on current state
		auto priorSolution = this->movingHead.getModel()->getDistortedMovingHeadSolution();

		// Perform the fit
		auto result = ofxCeres::Models::DistortedMovingHead::solve(targetPoints
			, panTiltSignal
			, priorSolution
			, this->solverSettings.getSolverSettings());
		//
		//--



		//--
		// Load solution into local parameters
		//
		this->movingHead.getModel()->setDistortedMovingHeadSolution(result.solution);

		{
			// Convert residual into degrees
			auto residual = result.residual;
			residual = sqrt(residual / targetPoints.size());
			residual = residual * RAD_TO_DEG;
			this->movingHead.getModel()->parameters.residual.set(residual);
		}
		//
		//--
	}

	//----------
	void
	Solver::solveGroup()
	{
		// Note this function uses the new MovingHeadGroup solver,
		// but feeds it only one moving head and fixes all the
		// markerPositions so that they cannot move

		auto selectedMarkers = Scene::X()->getMarkers()->getSelection();

		// Create the prior solution
		ofxCeres::Models::MovingHeadGroup::Solution priorSolution;
		{
			// Add the marker positions
			for (const auto& marker : selectedMarkers) {
				priorSolution.markerPositions.push_back(marker->position.get());
			}

			// Add this moving head
			priorSolution.movingHeads.push_back(this->movingHead.getModel()->getDistortedMovingHeadSolution());
		}

		// Create fixed constraints for all markers
		vector<shared_ptr<ofxCeres::Models::MovingHeadGroup::Constraint>> constraints;
		for (int i = 0; i < priorSolution.markerPositions.size(); i++) {
			auto constraint = make_shared<ofxCeres::Models::MovingHeadGroup::FixedMarkerConstraint>();
			constraint->markerIndex = i;
			constraints.push_back(constraint);
		}

		// A function to convert marker names into the index in our vector of selected markers
		auto getIndexForMarkerName = [&selectedMarkers](const string& name) {
			for (size_t i = 0; i < selectedMarkers.size(); i++) {
				if (selectedMarkers[i]->name.get() == name) {
					return i;
				}
			}
			throw(Exception("Couldn't find marker with name '" + name + "' in the selection."));
		};

		// Create the image
		ofxCeres::Models::MovingHeadGroup::Image image;
		auto calibrationPoints = this->calibrationPoints->getSelection();
		for (auto calibrationPoint : calibrationPoints) {
			image.panTiltSignal.push_back(calibrationPoint->panTiltSignal.get());
			image.markerIndex.push_back(getIndexForMarkerName(calibrationPoint->marker.get()));
		}

		// Create options (let's just keep defaults for this one - since it's not real group solve)
		ofxCeres::Models::MovingHeadGroup::Options options;
		{
			options.noDistortion = false;
		}

		// Solve
		auto result = ofxCeres::Models::MovingHeadGroup::solve(vector<ofxCeres::Models::MovingHeadGroup::Image>(1, image)
			, priorSolution
			, constraints
			, options
			, this->solverSettings.getSolverSettings());

		// Convert residual into meters
		{
			auto residual = result.residual;
			residual = sqrt(residual / image.panTiltSignal.size());
			this->movingHead.getModel()->parameters.residual.set(residual);
		}

		// Unpack the solution
		{
			this->movingHead.getModel()->setDistortedMovingHeadSolution(result.solution.movingHeads[0]);
		}
	}

	//---------
	shared_ptr<Data::CalibrationPointSet<DataPoint>>
	Solver::getCalibrationPoints()
	{
		return this->calibrationPoints;
	}

	//---------
	void
	Solver::getCalibrationData(vector<glm::vec3>& targetPoints
		, vector<glm::vec2>& panTiltSignal) const
	{
		auto markers = Scene::X()->getMarkers();
		auto dataPoints = this->calibrationPoints->getSelection();
		for (auto dataPoint : dataPoints) {
			auto marker = markers->getMarkerByName(dataPoint->marker.get());
			if (!marker) {
				dataPoint->setSelected(false);
			}
			else {
				targetPoints.push_back(marker->position.get());
				panTiltSignal.push_back(dataPoint->panTiltSignal.get());
			}
		}
	}

	//---------
	void
	Solver::prepareDataPoint(shared_ptr<DataPoint> dataPoint)
	{
		auto getResidualFunction = [this](DataPoint* dataPoint) {
			return this->getResidualOnDataPoint(dataPoint);
		};

		dataPoint->getResidualFunction = getResidualFunction;

		auto dataPointWeak = weak_ptr<DataPoint>(dataPoint);

		dataPoint->onTakeCurrent += [dataPointWeak, this]() {
			auto dataPoint = dataPointWeak.lock();
			dataPoint->panTiltSignal.set(this->movingHead.getCurrentPanTilt());
		};

		dataPoint->onGoValue += [dataPointWeak, this]() {
			auto dataPoint = dataPointWeak.lock();
			auto panTiltSignal = dataPoint->panTiltSignal.get();
			this->movingHead.parameters.pan.set(panTiltSignal.x);
			this->movingHead.parameters.tilt.set(panTiltSignal.y);
		};

		dataPoint->onGoPrediction += [dataPointWeak, this]() {
			auto dataPoint = dataPointWeak.lock();
			auto marker = Scene::X()->getMarkers()->getMarkerByName(dataPoint->marker);
			if (!marker) {
				dataPoint->setSelected(false);
			}
			else {
				this->movingHead.navigateToWorldTarget(marker->position.get());
			}
		};
	}

	//---------
	float
	Solver::getResidualOnDataPoint(DataPoint* dataPoint) const
	{
		auto transform = this->movingHead.getModel()->getTransform();

		//--
		//World -> Object space
		//

		//get marker position
		auto marker = Scene::X()->getMarkers()->getMarkerByName(dataPoint->marker);
		if (!marker) {
			throw(Exception("Marker " + dataPoint->marker.get() + " does not exist"));
		}

		//apply rigid body transform (marker into the object space of the moving head)
		auto targetInViewSpace4 = glm::inverse(transform) * glm::vec4(marker->position.get(), 1.0);
		targetInViewSpace4 /= targetInViewSpace4.w;
		auto targetInViewSpace = glm::vec3(targetInViewSpace4);

		if (targetInViewSpace == glm::vec3(0, 0, 0)) {
			//residual is 0 if dataPoint is coincident with fixture
			return 0.0f;
		}
		//
		//--

		// get ideal angles for target points
		auto idealAnglesForTarget = ofxCeres::VectorMath::getPanTiltToTargetInObjectSpace(targetInViewSpace);

		auto dataPointPanTiltIdeal = this->movingHead.getModel()->panTiltSignalToIdeal(dataPoint->panTiltSignal);

		auto disparity = ofxCeres::VectorMath::sphericalPolarDistance(idealAnglesForTarget, dataPointPanTiltIdeal);

		return disparity * RAD_TO_DEG;
	}

}