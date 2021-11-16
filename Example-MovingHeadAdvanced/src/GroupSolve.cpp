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

	// set default solver settings
	this->solverSettings.maxIterations.set(1000);
}

//----------
string
GroupSolve::getTypeName() const
{
	return "GroupSolve";
}

//----------
void
GroupSolve::update()
{
	if (this->parameters.solveContinuously) {
		this->solve();
	}
}
//----------
void
GroupSolve::drawWorld()
{
	if (this->isBeingInspected()) {
		auto movingHeads = Scene::X()->getMovingHeads();
		for (auto& movingHead : movingHeads) {
			movingHead.second->getSolver()->drawRaysAndResiduals();
		}
	}
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

	inspector->addParameterGroup(this->parameters);

	inspector->addButton("Prepare markers", 
		[this]() {
		this->prepareMarkers();
		});

	inspector->addSubMenu(this->solverSettings);
	inspector->addButton("Solve", [this]() {
		try {
			this->solve();
		}
		CATCH_TO_ALERT;
		}, OF_KEY_RETURN)->setHeight(100.0f);

}

//----------
void
GroupSolve::prepareMarkers()
{
	auto markers = this->scene.getMarkers();
	auto movingHeads = this->scene.getMovingHeads();

	auto selectedMarkers = markers->getSelection();
	for (auto marker : selectedMarkers) {
		auto markerName = marker->name.get();

		// Gather who can see this one
		set<int> movingHeadsThatSeeMarker;
		uint32_t movingHeadIndex = 0;
		for (auto movingHead : movingHeads) {
			auto calibrationPoints = movingHead.second->getSolver()->getCalibrationPoints()->getSelection();
			for (auto calibrationPoint : calibrationPoints) {
				if (calibrationPoint->marker.get() == markerName) {
					movingHeadsThatSeeMarker.insert(movingHeadIndex);
					break;
				}
			}
			movingHeadIndex++;
		}

		if (movingHeadsThatSeeMarker.empty()) {
			// Not seen - remove
			marker->setSelected(false);
		}
		else if (movingHeadsThatSeeMarker.size() == 1) {
			// Seen only once - fix
			auto constraint = marker->constraint.get();
			constraint.set(Marker::Constraint::Options::Fixed);
			marker->constraint.set(constraint);
		}
	}
}

//----------
void
GroupSolve::solve()
{
	auto markers = this->scene.getMarkers()->getSelection();
	map<int, set<int>> whereMarkersSeen; // <MarkerIndex, set<MovingHeadIndex>>

	// Gather moving heads
	vector<shared_ptr<DMX::MovingHead>> movingHeads;
	{
		const auto& movingHeadsByName = this->scene.getMovingHeads();
		for (auto it : movingHeadsByName) {
			movingHeads.push_back(it.second);
		}
	}

	// Get images
	vector<ofxCeres::Models::MovingHeadGroup::Image> images;
	{
		auto getIndexForMarkerName = [&markers](const string& name) {
			for (int i = 0; i < markers.size(); i++) {
				if (markers[i]->name.get() == name) {
					return i;
				}
			}

			// The marker doesn't exist - don't take this image
			// (e.g. we have a selected calibration point in the moving head
			// but deselected the corresponding marker because it's only seen once)
			return -1;
		};

		int movingHeadIndex = 0;
		for (const auto& movingHead : movingHeads) {
			ofxCeres::Models::MovingHeadGroup::Image image;
			auto calibrationPoints = movingHead->getSolver()->getCalibrationPoints()->getSelection();
			for (auto calibrationPoint : calibrationPoints) {
				auto markerIndex = getIndexForMarkerName(calibrationPoint->marker.get());
				if (markerIndex != -1) {
					image.panTiltSignal.push_back(calibrationPoint->panTiltSignal.get());
					image.markerIndex.push_back(markerIndex);
					whereMarkersSeen[markerIndex].insert(movingHeadIndex);
				}
			}
			if (image.markerIndex.empty()) {
				throw("No good calibration points for moving head #" + ofToString(movingHeadIndex));
			}
			images.push_back(image);
			movingHeadIndex++;
		}
	}

	// Get initial solution
	ofxCeres::Models::MovingHeadGroup::Solution initialSolution;
	{
		for (const auto & marker : markers) {
			initialSolution.markerPositions.push_back(marker->position.get());
		}
		for (const auto& movingHead : movingHeads) {
			initialSolution.movingHeads.push_back(movingHead->getModel()->getDistortedMovingHeadSolution());
		}
	}

	// Get constraints
	vector<shared_ptr<ofxCeres::Models::MovingHeadGroup::Constraint>> constraints;
	{
		vector<int> fixedMarkers;
		vector<int> planeMarkers;

		uint32_t markerIndex = 0;
		for (auto marker : markers) {
			switch (marker->constraint.get()) {
			case Marker::Constraint::Free:
			{
				// If it's free, we have to check that it's seen in at least 2 views
				uint32_t countOfMovingHeadsThatSawThisPoint = 0;
				for (const auto & movingHead : movingHeads) {
					auto calibrationPoints = movingHead->getSolver()->getCalibrationPoints()->getSelection();
					bool seenInThisMovingHead = false;
					for (auto calibrationPoint : calibrationPoints) {
						if (calibrationPoint->marker.get() == marker->name.get()) {
							seenInThisMovingHead = true;
							break;
						}
					}
					if (seenInThisMovingHead) {
						countOfMovingHeadsThatSawThisPoint++;
					}
				}
				if (countOfMovingHeadsThatSawThisPoint < 2) {
					throw(Exception("Marker '" + marker->name.get() + "' is set to Free but only " + ofToString(countOfMovingHeadsThatSawThisPoint) + " moving heads saw this marker. All free markers must be seen in at least 2 moving heads."));
				}

				break;
			}
			case Marker::Constraint::Fixed:
			{
				// Record fixed markers
				fixedMarkers.push_back(markerIndex);

				// Create the constraint
				auto constraint = make_shared<ofxCeres::Models::MovingHeadGroup::FixedMarkerConstraint>();
				constraint->markerIndex = markerIndex;
				constraints.push_back(constraint);
				break;
			}
			case Marker::Constraint::Plane:
			{
				// Record for later (do these at the end because we need the fixed markers also)
				planeMarkers.push_back(markerIndex);
				break;
			}
			default:
				break;
			}
			markerIndex++;
		}

		// Create the plane marker constraints
		for (auto& planeMarkerIndex : planeMarkers) {
			// Check we have at least 2 fixed markers
			auto marker = markers[planeMarkerIndex];
			if (fixedMarkers.size() < 2) {
				throw(Exception("Marker '" + marker->name.get() + "' is set to Plane which means that at least 2 other markers should be set to Fixed."));
			}

			// Get the plane that it will be constrained to
			auto pointA = markers[fixedMarkers[0]]->position.get();
			auto pointB = markers[fixedMarkers[1]]->position.get();
			auto pointC = marker->position.get();

			auto normal = glm::normalize(glm::cross(pointB - pointA, pointC - pointA));
			// abcd . point, 1 = 0
			// normal.x * point.x + normal.y * point.y + normal.z * point.z + d = 0
			auto d = -glm::dot(normal, pointC);

			auto constraint = make_shared<ofxCeres::Models::MovingHeadGroup::MarkerInPlaneConstraint>();
			constraint->markerIndex = planeMarkerIndex;
			constraint->abcd = glm::vec4{
				normal.x
				, normal.y
				, normal.z
				, d
			};
			constraints.push_back(constraint);
		}
	}

	// Check and non-Fixed markers are seen in at least 2 views
	{
		for (size_t i = 0; i < markers.size(); i++) {
			auto marker = markers[i];
			if (marker->constraint.get() == Marker::Constraint::Fixed) {
				continue;
			}

			auto viewCount = whereMarkersSeen[i].size();
			if (viewCount < 2) {
				throw(Exception("Marker '" + marker->name.get() + "' with constraint "
					+ marker->constraint.get().toString() + " is only seen in "
					+ ofToString(viewCount) + " views (2 minimum)"));
			}
		}
	}

	// Get options
	ofxCeres::Models::MovingHeadGroup::Options options;
	{
		options.noDistortion = this->parameters.noDistortion.get();
	}

	// Get solver settings
	auto solverSettings = this->solverSettings.getSolverSettings();

	// Perform the fit
	auto result = ofxCeres::Models::MovingHeadGroup::solve(images
		, initialSolution
		, constraints
		, options
		, solverSettings);

	// Unpack the solution
	{
		for (int i = 0; i < movingHeads.size(); i++) {
			movingHeads[i]->getModel()->setDistortedMovingHeadSolution(result.solution.movingHeads[i]);
		}
		for (int i = 0; i < markers.size(); i++) {
			markers[i]->position.set(result.solution.markerPositions[i]);
		}
	}

	// Mark datapoints as needing calculate residual
	{
		for (auto movingHead : movingHeads) {
			movingHead->getSolver()->markResidualsStale();
		}
	}

	// Solve the focus models if converged
	if (result.isConverged()) {
		for (auto movingHead : movingHeads) {
			movingHead->getSolver()->solveFocus();
		}
	}

	// Turn off continuous solve when we have a solution
	if (result.isConverged()) {
		this->parameters.solveContinuously.set(false);
	}
}