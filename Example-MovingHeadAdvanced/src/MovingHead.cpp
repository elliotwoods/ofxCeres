#include "pch_ofApp.h"
#include "MovingHead.h"
#include "Widgets/PanTiltTrackpad.h"

//---------
MovingHead::MovingHead() {
	RULR_SERIALIZE_LISTENERS;
}

//---------
string MovingHead::getTypeName() const {
	return "MovingHead";
}

//---------
void MovingHead::update() {
	// Update the range of pan-tilt
	this->currentPanTiltSignal.setMin({ this->fixtureSettings.panRange.get().x, this->fixtureSettings.tiltRange.get().x });
	this->currentPanTiltSignal.setMax({ this->fixtureSettings.panRange.get().y, this->fixtureSettings.tiltRange.get().y });
}

//---------
void MovingHead::drawWorld(bool selected) {
	ofPushStyle();
	{
		auto calibrationPoints = this->calibrationPoints.getSelection();

		//draw target points
		if (selected) {
			for (auto calibrationPoint : calibrationPoints) {
				calibrationPoint->drawWorld();
			}
		}

		auto transform = this->getTransform();

		//draw moving head body
		ofNoFill();
		vector<glm::vec3> transmissionsInObjectSpace;
		ofPushMatrix();
		{
			ofMultMatrix(transform);

			//draw hardware
			{
				ofSetColor(selected ? 220 : 100);
				ofDrawAxis(0.4f);

				// base
				ofDrawBox(glm::vec3(0, -0.35, 0), 0.5, 0.1, 0.4);

				auto panTiltIdeal = this->panTiltSignalToIdeal(this->currentPanTiltSignal);

				// Axis 1
				ofPushMatrix();
				{
					ofRotateDeg(- panTiltIdeal.x, 0, 1, 0);

					ofDrawBox({ -0.15, -0.3 + 0.35f / 2.0f, 0 }, 0.075, 0.35, 0.2);
					ofDrawBox({ +0.15, -0.3 + 0.35f / 2.0f, 0 }, 0.075, 0.35, 0.2);

					// Axis 2
					ofPushMatrix();
					{
						ofRotateDeg(panTiltIdeal.y + this->calibrationParameters.tiltOffset.get(), 1, 0, 0);

						// head
						ofPushMatrix();
						{
							ofSetSphereResolution(6);
							ofScale(0.1, 0.2, 0.1);
							ofDrawSphere(1.0);
						}
						ofPopMatrix();

						// beam
						ofPushMatrix();
						{
							ofTranslate(0.0f, 0.05 + 0.5f / 2.0f, 0.0f);
							ofSetColor(selected ? 255 : 100);
							ofDrawCylinder(0.02, 0.5f);
						}
						ofPopMatrix();
					}
					ofPopMatrix();
				}
				ofPopMatrix();
			}
		}
		ofPopMatrix();

		if (selected) {
			//draw selected data point
			auto focusedDataPoint = this->focusedDataPoint.lock();
			if (focusedDataPoint) {
				ofPushStyle();
				{
					ofNoFill();
					ofSetColor(255);
					ofDrawCircle(focusedDataPoint->targetPoint.get(), 0.05f);
				}
				ofPopStyle();
			}

			//draw rays and residuals
			ofPushStyle();
			{
				for (const auto & calibrationPoint : calibrationPoints) {
					ofSetColor(calibrationPoint->color);

					auto idealPanTilt = this->panTiltSignalToIdeal(calibrationPoint->panTiltSignal.get());
					auto transmissionObject = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt(idealPanTilt
						, this->calibrationParameters.tiltOffset.get());

					auto transmissionWorld4 = transform * glm::vec4(transmissionObject, 1.0f);
					auto transmissionWorld = (glm::vec3) (transmissionWorld4 / transmissionWorld4.w);

					ofSetLineWidth(2.0f);
					ofDrawLine(this->calibrationParameters.translation.get(), transmissionWorld);
					ofDrawBitmapString(calibrationPoint->name.get(), transmissionWorld);

					ofSetLineWidth(1.0f);
					ofSetColor(100);
					ofDrawLine(transmissionWorld, calibrationPoint->targetPoint);
				}
			}
			ofPopStyle();
		}
	}
	ofPopStyle();
}

//---------
void MovingHead::serialize(nlohmann::json & json) {
	nlohmann::json jsonCalibrationPoints;
	this->calibrationPoints.serialize(jsonCalibrationPoints);
	json["calibrationPoints"] = jsonCalibrationPoints;
	
	{
		json["beamParameters"] << this->beamParameters.focus;
	}

	{
		json["fitParameters"] << this->fitParameters.distortionEnabled;
	}

	{
		json["calibrationParameters"] << this->calibrationParameters.translation;
		json["calibrationParameters"] << this->calibrationParameters.rotationVector;
		json["calibrationParameters"] << this->calibrationParameters.tiltOffset;
		json["calibrationParameters"] << this->calibrationParameters.panDistortion;
		json["calibrationParameters"] << this->calibrationParameters.tiltDistortion;
		json["calibrationParameters"] << this->calibrationParameters.residual;
	}

	// fixtureSettings
	{
		json["fixtureSettings"] << this->fixtureSettings.panRange;
		json["fixtureSettings"] << this->fixtureSettings.tiltRange;
		json["fixtureSettings"] << this->fixtureSettings.dmxPanPolarity;

		// dmxAddresses
		{
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.dmxStartAddress;
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.panCoarse;
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.panFine;
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.tiltCoarse;
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.tiltFine;
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.brightness;
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.focusCoarse;
			json["fixtureSettings"]["dmxAddresses"] << this->fixtureSettings.dmxAddresses.focusFine;
		}
	}
}

//---------
void MovingHead::deserialize(const nlohmann::json & json) {
	if (json.count("calibrationPoints") != 0) {
		this->calibrationPoints.deserialize(json["calibrationPoints"]);

		// Setup callbacks
		auto calibrationPoints = this->calibrationPoints.getAllCaptures();
		for (auto calibrationPoint : calibrationPoints) {
			this->prepareDataPoint(calibrationPoint);
		}
	}

	if (json.count("beamParameters") != 0) {
		json["beamParameters"] >> this->beamParameters.focus;
	}

	if(json.count("fitParameters") != 0) {
		json["fitParameters"] >> this->fitParameters.distortionEnabled;
	}

	if (json.count("calibrationParameters") != 0) {
		json["calibrationParameters"] >> this->calibrationParameters.translation;
		json["calibrationParameters"] >> this->calibrationParameters.rotationVector;
		json["calibrationParameters"] >> this->calibrationParameters.tiltOffset;
		json["calibrationParameters"] >> this->calibrationParameters.panDistortion;
		json["calibrationParameters"] >> this->calibrationParameters.tiltDistortion;
		json["calibrationParameters"] >> this->calibrationParameters.residual;
	}

	// fixtureSettings
	if(json.count("fixtureSettings") != 0) {
		json["fixtureSettings"] >> this->fixtureSettings.panRange;
		json["fixtureSettings"] >> this->fixtureSettings.tiltRange;
		json["fixtureSettings"] >> this->fixtureSettings.dmxPanPolarity;

		// dmxAddresses
		if (json["fixtureSettings"].count("dmxAddresses") != 0) {
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.dmxStartAddress;
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.panCoarse;
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.panFine;
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.tiltCoarse;
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.tiltFine;
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.brightness;
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.focusCoarse;
			json["fixtureSettings"]["dmxAddresses"] >> this->fixtureSettings.dmxAddresses.focusFine;
		}
	}
}

//---------
void MovingHead::populateWidgets(shared_ptr<ofxCvGui::Panels::Widgets> widgets) {
	
	// make the trackpad and set it up
	{
		auto trackpadWidget = make_shared<Widgets::PanTiltTrackpad>(this->currentPanTiltSignal);
		auto trackpadWidgetWeak = weak_ptr<Widgets::PanTiltTrackpad>(trackpadWidget);

		trackpadWidget->onDraw += [this, trackpadWidgetWeak](ofxCvGui::DrawArguments & args) {
			auto trackpadWidget = trackpadWidgetWeak.lock();

			// draw the existing selected data points onto the trackpad
			ofMesh pointsPreview;
			auto calibrationPoints = this->calibrationPoints.getSelection();
			for (auto calibrationPoint : calibrationPoints) {
				pointsPreview.addColor(calibrationPoint->color.get());
				pointsPreview.addVertex(glm::vec3(trackpadWidget->toXY(calibrationPoint->panTiltSignal.get()), 0.0f));

				if (focusedDataPoint.lock() == calibrationPoint) {
					ofPushStyle();
					{
						ofDrawCircle(trackpadWidget->toXY(calibrationPoint->panTiltSignal.get()), 3.0f);
					}
					ofPopStyle();
				}
			}
			pointsPreview.drawVertices();
		};

		trackpadWidget->onMouse += [this, trackpadWidgetWeak](ofxCvGui::MouseArguments & args) {
			// Focus the data point next to cursor
			auto trackpadWidget = trackpadWidgetWeak.lock();

			// Check mouse is inside widget
			if (trackpadWidget->isMouseOver()) {

				// search for closest dataPoint
				auto minDistance2 = std::numeric_limits<float>::max();
				auto calibrationPoints = this->calibrationPoints.getSelection();

				for (auto calibrationPoint : calibrationPoints) {
					auto drawnPosition = trackpadWidget->toXY(calibrationPoint->panTiltSignal.get());
					auto distance2 = glm::distance2(args.local, drawnPosition);
					if (distance2 < minDistance2 && sqrt(distance2) < 30) {
						minDistance2 = distance2;
						this->focusedDataPoint = calibrationPoint;
					}
				}
			}
		};
		widgets->add(trackpadWidget);
	}

	widgets->add(make_shared<ofxCvGui::Widgets::EditableValue<glm::vec2>>(this->currentPanTiltSignal));

	widgets->addTitle("Data", ofxCvGui::Widgets::Title::Level::H2);
	{
		widgets->addButton("Add new data point...", [this]() {
			auto newDataPoint = make_shared<Data::MovingHeadDataPoint>();
			newDataPoint->name = ofSystemTextBoxDialog("Name");
			newDataPoint->panTiltSignal = this->currentPanTiltSignal.get();

			auto targetPoint = this->lastWorldPosition;
			{
				auto response = ofSystemTextBoxDialog("Target point in world [" + ofToString(targetPoint) + "]", ofToString(targetPoint));

				if (!response.empty()) {
					stringstream ss(response);
					ss >> targetPoint;
				}
			}

			newDataPoint->targetPoint = targetPoint;

			this->prepareDataPoint(newDataPoint);
			this->calibrationPoints.add(newDataPoint);
		});

		this->calibrationPoints.populateWidgets(widgets, false);

		widgets->addButton("Add test data", [this]() {
			this->addTestData();
		});

		widgets->addTitle("Focused", ofxCvGui::Widgets::Title::Level::H3);
		{
			widgets->addLiveValue<string>("Name", [this]() {
				auto focusedDataPoint = this->focusedDataPoint.lock();
				if (focusedDataPoint) {
					return focusedDataPoint->name.get();
				}
				else {
					return (string) "";
				}
			});

			widgets->addToggle("Selected", [this]() {
				auto focusedDataPoint = this->focusedDataPoint.lock();
				if (focusedDataPoint) {
					return focusedDataPoint->isSelected();
				}
				return false;
			}, [this](bool value) {
				auto focusedDataPoint = this->focusedDataPoint.lock();
				if (focusedDataPoint) {
					focusedDataPoint->setSelected(value);
				}
			})->setHotKey('s');

			widgets->addButton("GO to value", [this]() {
				auto focusedDataPoint = this->focusedDataPoint.lock();
				if (focusedDataPoint) {
					focusedDataPoint->onGoValue.notifyListeners();
				}
			})->setHotKey('g');

			widgets->addButton("GO to prediction", [this]() {
				auto focusedDataPoint = this->focusedDataPoint.lock();
				if (focusedDataPoint) {
					focusedDataPoint->onGoPrediction.notifyListeners();
				}
			})->setHotKey('p');
		}
	}

	widgets->addTitle("Beam parameters", ofxCvGui::Widgets::Title::Level::H2);
	{
		widgets->addSlider(this->beamParameters.focus);
	}


	widgets->addTitle("Calibration", ofxCvGui::Widgets::Title::Level::H2);
	{
		widgets->addToggle(this->fitParameters.distortionEnabled);

		widgets->addButton("Solve", [this]() {
			this->solve();
		}, OF_KEY_RETURN)->setHeight(100.0f);

		widgets->addEditableValue<glm::vec3>(this->calibrationParameters.translation);
		widgets->addEditableValue<glm::vec3>(this->calibrationParameters.rotationVector);
		widgets->addSlider(this->calibrationParameters.tiltOffset);
		widgets->addEditableValue<glm::vec3>(this->calibrationParameters.panDistortion);
		widgets->addEditableValue<glm::vec3>(this->calibrationParameters.tiltDistortion);
		widgets->addLiveValue<float>(this->calibrationParameters.residual);

		widgets->addButton("Reset", [this]() {
			this->calibrationParameters.translation.set(glm::vec3(0.0));
			this->calibrationParameters.rotationVector.set(glm::vec3(0.0));
			this->calibrationParameters.tiltOffset.set(0.0);
			this->calibrationParameters.panDistortion.set({ 0, 1, 0 });
			this->calibrationParameters.tiltDistortion.set({ 0, 1, 0 });
			this->calibrationParameters.residual.set(0.0);
		});

		widgets->addButton("Focus datapoint with highest residual", [this]() {
			this->focusDataPointWithHighestResidual();
		});
	}

	widgets->addTitle("Fixture settings", ofxCvGui::Widgets::Title::Level::H2);
	{
		widgets->addEditableValue<glm::vec2>(this->fixtureSettings.panRange);
		widgets->addEditableValue<glm::vec2>(this->fixtureSettings.tiltRange);
		{
			auto selector = widgets->addMultipleChoice("DMX pan polarity");
			selector->addOptions({ "Right (+)", "Left (+)" });
			selector->entangle(this->fixtureSettings.dmxPanPolarity);
		}

		widgets->addTitle("DMX Addresses", ofxCvGui::Widgets::Title::Level::H3);
		{
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.dmxStartAddress);
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.panCoarse);
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.panFine);
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.tiltCoarse);
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.tiltFine);
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.brightness);
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.focusCoarse);
			widgets->addEditableValue<uint16_t>(this->fixtureSettings.dmxAddresses.focusFine);
		}
	}

	widgets->addTitle("Data fudge", ofxCvGui::Widgets::Title::Level::H2);
	{
		widgets->addButton("Scale pan values", [this]() {
			auto response = ofSystemTextBoxDialog("Scale pan values", "1.0");
			if (!response.empty()) {
				auto scale = ofToFloat(response);
				auto calibrationPoints = this->calibrationPoints.getSelection();
				for (auto calibrationPoint : calibrationPoints) {
					auto panTiltValue = calibrationPoint->panTiltSignal.get();
					panTiltValue.x *= scale;
					calibrationPoint->panTiltSignal.set(panTiltValue);
				}
			}
		});

		widgets->addButton("Scale tilt values", [this]() {
			auto response = ofSystemTextBoxDialog("Scale tilt values", "1.0");
			if (!response.empty()) {
				auto scale = ofToFloat(response);
				auto calibrationPoints = this->calibrationPoints.getSelection();
				for (auto calibrationPoint : calibrationPoints) {
					auto panTiltValue = calibrationPoint->panTiltSignal.get();
					panTiltValue.y *= scale;
					calibrationPoint->panTiltSignal.set(panTiltValue);
				}
			}
		});
	}
}

//---------
shared_ptr<ofxCvGui::Panels::Widgets> MovingHead::getListPanel() {
	return this->calibrationPoints.getListPanel();
}


//---------
void MovingHead::solve() {
	
	//--
	// Prepare data
	//
	vector<glm::vec3> targetPoints;
	vector<glm::vec2> panTiltAngles;

	auto calibrationPoints = this->calibrationPoints.getSelection();
	for (auto calibrationPoint : calibrationPoints) {
		targetPoints.push_back(calibrationPoint->targetPoint);
		panTiltAngles.push_back(calibrationPoint->panTiltSignal);
	}
	//
	//--

	if (this->fitParameters.distortionEnabled.get()) {
		//--
		// Perform fit
		//
		auto priorSolution = ofxCeres::Models::DistortedMovingHead::Solution();
		priorSolution.basicSolution = ofxCeres::Models::MovingHead::Solution{
			this->calibrationParameters.translation.get()
				, this->calibrationParameters.rotationVector.get()
				, this->calibrationParameters.tiltOffset.get()
		};

		// we don't initialise the distortion parameters, we always begin fit with undistorted

		auto result = ofxCeres::Models::DistortedMovingHead::solve(targetPoints
			, panTiltAngles
			, priorSolution);
		//
		//--



		//--
		// Load solution into local parameters
		//
		this->calibrationParameters.translation = result.solution.basicSolution.translation;
		this->calibrationParameters.rotationVector = result.solution.basicSolution.rotationVector;
		this->calibrationParameters.tiltOffset = result.solution.basicSolution.tiltOffset;
		this->calibrationParameters.panDistortion.set({
			result.solution.panDistortion[0]
			, result.solution.panDistortion[1]
			, result.solution.panDistortion[2]
			});
		this->calibrationParameters.tiltDistortion.set({
			result.solution.tiltDistortion[0]
			, result.solution.tiltDistortion[1]
			, result.solution.tiltDistortion[2]
			});

		{
			auto residual = result.residual;
			residual -= result.solution.panDistortion[0] * result.solution.panDistortion[0] / 100.0f;
			residual -= result.solution.tiltDistortion[0] * result.solution.tiltDistortion[0] / 100.0f;
			residual = sqrt(residual / targetPoints.size());
			residual = residual * RAD_TO_DEG;
			this->calibrationParameters.residual.set(residual);
		}

		//
		//--
	}
	else {
		auto priorSolution = ofxCeres::Models::MovingHead::Solution{
			this->calibrationParameters.translation.get()
				, this->calibrationParameters.rotationVector.get()
				, this->calibrationParameters.tiltOffset.get()
		};

		auto result = ofxCeres::Models::MovingHead::solve(targetPoints
			, panTiltAngles
			, priorSolution);

		this->calibrationParameters.translation = result.solution.translation;
		this->calibrationParameters.rotationVector = result.solution.rotationVector;
		this->calibrationParameters.tiltOffset = result.solution.tiltOffset;
		this->calibrationParameters.panDistortion.set({
			0
			, 1
			, 0
			});
		this->calibrationParameters.tiltDistortion.set({
			0
			, 1
			, 0
			});
		this->calibrationParameters.residual.set(sqrt(result.residual / targetPoints.size()) * RAD_TO_DEG);
	}
}

//---------
void MovingHead::addTestData() {
	vector<string> names({
		"A"
		, "C"
		, "D"
		, "Y"
		, "E"
		, "F"
		, "G"

		, "B_A0"
		, "B_A1"
		, "B_A2"
		, "B_B0"
		, "B_B1"
		, "B_B2"
		, "B_C0"
		, "B_C1"
		, "B_C2"

		, "O_A0"
		, "O_A1"
		, "O_A2"
		, "O_B0"
		, "O_B1"
		, "O_B2"
		, "O_C0"
		, "O_C1"
		, "O_C2"

		, "P_A0"
		, "P_A1"
		, "P_A2"
		, "P_B0"
		, "P_B1"
		, "P_B2"
		, "P_C0"
		, "P_C1"
		, "P_C2"
		});

	vector<glm::vec4> dmxValues({
			{203, 112, 21, 40}
			, {161, 47, 22, 20}
			, {151, 16, 22, 39}
			, {0, 0, 22, 19}
			, {151, 40, 59, 5} // F and E are switched
			, {151, 16, 3, 5} // F and E are switched
			, {203, 105, 41, 26}

			, {184 , 0 , 21 , 43}
			, {184 , 0 , 28 , 47}
			, {184 , 0 , 35 , 47}
			, {181 , 30 , 21 , 48}
			, {181 , 35 , 28 , 63}
			, {181 , 36 , 35 , 72}
			, {178 , 36 , 21 , 52}
			, {178 , 36 , 28 , 75}
			, {178 , 36 , 36 , 52}

			, {173 , 90 , 22 , 0}
			, {173 , 90 , 26 , 0}
			, {173 , 92 , 29 , 59}
			, {172 , 19 , 21 , 65}
			, {172 , 17 , 25 , 59}
			, {172 , 17 , 29 , 59}
			, {170 , 37 , 21 , 64}
			, {170 , 31 , 25 , 65}
			, {170 , 36 , 29 , 59}

			, {202 , 123 , 21 , 44}
			, {203 , 21 , 25 , 21}
			, {203 , 20 , 28 , 55}
			, {201 , 93 , 21 , 44}
			, {201 , 86 , 25 , 11}
			, {201 , 86 , 28 , 46}
			, {200 , 65 , 21 , 44}
			, {200 , 63 , 25 , 6}
			, {200 , 61 , 28 , 34}
		}
	);

	vector<glm::vec3> targetPoints({
			{-2.20, -2.56, 0.78}
			, {3.76, 0.00, 0.78}
			, {3.76, -2.45, 0.78}
			, {2.35, 0.00, 0.78}
			, {3.76, -2.45, 2.32}
			, {3.76, -2.45, 0}
			, {-2.20, -2.56, 2.19}

			, {0.00, 0.00, 0.78}
			, {0.00, 0.00, 1.28}
			, {0.00, 0.00, 1.78}
			, {0.50, 0.00, 0.78}
			, {0.50, 0.00, 1.28}
			, {0.50, 0.00, 1.78}
			, {1.00, 0.00, 0.78}
			, {1.00, 0.00, 1.28}
			, {1.00, 0.00, 1.78}

			, {1.73, 0.00, 0.78}
			, {1.73, 0.00, 1.03}
			, {1.73, 0.00, 1.28}
			, {1.98, 0.00, 0.78}
			, {1.98, 0.00, 1.03}
			, {1.98, 0.00, 1.28}
			, {2.23, 0.00, 0.78}
			, {2.23, 0.00, 1.03}
			, {2.23, 0.00, 1.28}

			, {-2.20, -2.39, 0.78}
			, {-2.20, -2.39, 1.03}
			, {-2.20, -2.39, 1.28}
			, {-2.20, -2.14, 0.78}
			, {-2.20, -2.14, 1.03}
			, {-2.20, -2.14, 1.28}
			, {-2.20, -1.89, 0.78}
			, {-2.20, -1.89, 1.03}
			, {-2.20, -1.89, 1.28}
		});	

	// SketchUp to openFrameworks coordinates
	for (auto & targetPoint : targetPoints) {
		targetPoint = glm::vec3(targetPoint.x, targetPoint.z, -targetPoint.y);
	}

	// Convert DMX into pan-tilt angles
	vector<glm::vec2> panTiltAngles;
	for (const auto & dmxValue : dmxValues) {
		uint16_t panTotal = (uint16_t)dmxValue[0] << 8;
		panTotal += (uint16_t)dmxValue[1];

		uint16_t tiltTotal = (uint16_t)dmxValue[2] << 8;
		tiltTotal += (uint16_t)dmxValue[3];

		// WARNING : flip at the DMX level if the polarity is inversed
		if (this->fixtureSettings.dmxPanPolarity.get() == 1) {
			panTotal = std::numeric_limits<uint16_t>::max() - panTotal;
		}

		panTiltAngles.push_back({			
			ofMap(panTotal
				, 0
				, std::numeric_limits<uint16_t>::max()
				, this->fixtureSettings.panRange.get().x
				, this->fixtureSettings.panRange.get().y)

			, ofMap(tiltTotal
				, 0
				, std::numeric_limits<uint16_t>::max()
				, this->fixtureSettings.tiltRange.get().x
				, this->fixtureSettings.tiltRange.get().y)
			});
	}

	for (size_t i = 0; i < names.size(); i++) {
		auto newDataPoint = make_shared<Data::MovingHeadDataPoint>();
		newDataPoint->name = names[i];
		newDataPoint->panTiltSignal = panTiltAngles[i];
		newDataPoint->targetPoint = targetPoints[i];

		this->prepareDataPoint(newDataPoint);

		this->calibrationPoints.add(newDataPoint);
	}
}

//---------
glm::mat4 MovingHead::getTransform() const {
	return ofxCeres::VectorMath::createTransform(this->calibrationParameters.translation.get()
		, this->calibrationParameters.rotationVector.get());
}

//---------
glm::vec2 MovingHead::getPanTiltForWorldTarget(const glm::vec3 & world
	, const glm::vec2 & currentPanTilt) const {
	auto objectSpacePosition4 = glm::inverse(this->getTransform()) * glm::vec4(world, 1.0f);
	auto objectSpacePosition = (glm::vec3) (objectSpacePosition4 / objectSpacePosition4.w);

	auto panTiltObject = ofxCeres::VectorMath::getPanTiltToTargetInObjectSpace(objectSpacePosition, 0.0f);
	glm::vec2 axisOffset = glm::vec2(0, -this->calibrationParameters.tiltOffset.get());

	// build up the options
	vector<glm::vec2> panTiltOptions;
	{
		// basic option
		panTiltOptions.push_back(panTiltObject + axisOffset);

		// alternative pan options
		{
			for (float pan = panTiltObject.x - 360.0f; pan >= this->fixtureSettings.panRange.get().x; pan -= 360.0f) {
				panTiltOptions.push_back(glm::vec2(pan, panTiltObject.y) + axisOffset);
			}
			for (float pan = panTiltObject.x + 360.0f; pan <= this->fixtureSettings.panRange.get().y; pan += 360.0f) {
				panTiltOptions.push_back(glm::vec2(pan, panTiltObject.y) + axisOffset);
			}
		}

		// flipped tilt options
		{
			for (float pan = panTiltObject.x - 180.0f; pan >= this->fixtureSettings.panRange.get().x; pan -= 360.0f) {
				panTiltOptions.push_back(glm::vec2(pan, -panTiltObject.y) + axisOffset);
			}
			for (float pan = panTiltObject.x + 180.0f; pan <= this->fixtureSettings.panRange.get().y; pan += 360.0f) {
				panTiltOptions.push_back(glm::vec2(pan, -panTiltObject.y) + axisOffset);
			}
		}
	}
	
	// search through options for closest one
	auto minDistance2 = std::numeric_limits<float>::max();
	glm::vec2 bestOption = panTiltObject + axisOffset;
	for (const auto & panTiltOption : panTiltOptions) {
		auto distance2 = glm::distance2(panTiltOption, currentPanTilt);
		if (distance2 < minDistance2) {
			bestOption = panTiltOption;
			minDistance2 = distance2;
		}
	}

	// apply distortion
	auto signalPanTilt = this->panTiltIdealToSignal(bestOption);

	// presumes we are inside range (TODO : in edge cases, these values might be outside range because of distortion)
	return signalPanTilt;
}

//---------
void MovingHead::navigateToWorldTarget(const glm::vec3 & world) {
	// Navigate pan-tilt values
	auto panTiltAngles = this->getPanTiltForWorldTarget(world, this->currentPanTiltSignal.get());
	this->currentPanTiltSignal.set(panTiltAngles);

	// Navigate the focus (NOTE : hardcoded for Robe Pointe)
	{
		//check 'Fit focus values.ipynb'
		auto distance = glm::distance(world, this->calibrationParameters.translation.get());

		if (distance < 1.5f) {
			// near range
			this->beamParameters.focus.set(1.0f);
		}
		else {
			auto inverseDistance = 1.0f / distance;
			auto dmxCoarseFloat =
				-77.16738954 * inverseDistance * inverseDistance * inverseDistance
				+ -45.98512483 * inverseDistance * inverseDistance
				+ 403.21323162 * inverseDistance
				+ 29.67284246;
			this->beamParameters.focus.set(dmxCoarseFloat / 256.0f);
		}
	}
}

//---------
void MovingHead::setWorldCursorPosition(const glm::vec3 & position) {
	// Focus the data point close to the world cursor
	auto minDistance2 = numeric_limits<float>::max();
	auto dataPoints = this->calibrationPoints.getAllCaptures();
	for (auto dataPoint : dataPoints) {
		auto distance2 = glm::distance2(dataPoint->targetPoint.get(), position);

		// take the point as the focus if it's closest to the cursor so far (within 30cm)
		if (distance2 < minDistance2 && sqrt(distance2) < 0.3f) {
			this->focusedDataPoint = dataPoint;
			minDistance2 = distance2;
		}
	}
}

//---------
void MovingHead::renderDMX(vector<uint8_t> & dmxValues) const {
	auto panSignalValue = (uint16_t) ofMap(this->currentPanTiltSignal.get().x
		, this->currentPanTiltSignal.getMin().x
		, this->currentPanTiltSignal.getMax().x
		, 0
		, std::numeric_limits<uint16_t>::max());

	if (this->fixtureSettings.dmxPanPolarity.get() == 1) {
		panSignalValue = std::numeric_limits<uint16_t>::max() - panSignalValue;
	}

	auto tiltSignalValue = (uint16_t) ofMap(this->currentPanTiltSignal.get().y
		, this->currentPanTiltSignal.getMin().y
		, this->currentPanTiltSignal.getMax().y
		, 0
		, std::numeric_limits<uint16_t>::max());

	auto dmxOffset = this->fixtureSettings.dmxAddresses.dmxStartAddress.get() - 1;

	dmxValues[dmxOffset + this->fixtureSettings.dmxAddresses.panCoarse.get()] = panSignalValue >> 8;
	dmxValues[dmxOffset + this->fixtureSettings.dmxAddresses.panFine.get()] = panSignalValue & 255;

	dmxValues[dmxOffset + this->fixtureSettings.dmxAddresses.tiltCoarse.get()] = tiltSignalValue >> 8;
	dmxValues[dmxOffset + this->fixtureSettings.dmxAddresses.tiltFine.get()] = tiltSignalValue & 255;

	dmxValues[dmxOffset + this->fixtureSettings.dmxAddresses.brightness.get()] = 255;

	//focus
	{
		auto focusSignalFloat = floor(this->beamParameters.focus.get() * (std::numeric_limits<uint16_t>::max()- 1));
		auto focusSignal = (uint16_t)focusSignalFloat;

		auto focusCoarse = (uint8_t)(focusSignal >> 8);
		auto focusFine = (uint8_t)(focusSignal & 255);
		dmxValues[dmxOffset + this->fixtureSettings.dmxAddresses.focusCoarse.get()] = focusCoarse;
		dmxValues[dmxOffset + this->fixtureSettings.dmxAddresses.focusCoarse.get()] = focusFine;
	}
}

//---------
glm::vec2 MovingHead::panTiltIdealToSignal(const glm::vec2 & ideal) const {
	auto panOptions = ofxCeres::VectorMath::powerSeries2Inverse(ideal.x, (float*)& this->calibrationParameters.panDistortion.get());
	auto tiltOptions = ofxCeres::VectorMath::powerSeries2Inverse(ideal.y, (float*)& this->calibrationParameters.tiltDistortion.get());

	auto signal = glm::vec2 {
		ofxCeres::VectorMath::pickClosest(ideal.x, panOptions.first, panOptions.second)
		, ofxCeres::VectorMath::pickClosest(ideal.y, tiltOptions.first, tiltOptions.second)
	};

	auto recalcIdeal = this->panTiltSignalToIdeal(signal);
	return signal;
}

//---------
glm::vec2 MovingHead::panTiltSignalToIdeal(const glm::vec2 & signal) const {
	return {
		ofxCeres::VectorMath::powerSeries2(signal.x, (float*)& this->calibrationParameters.panDistortion.get())
		, ofxCeres::VectorMath::powerSeries2(signal.y, (float*)& this->calibrationParameters.tiltDistortion.get())
	};
}

//---------
void MovingHead::prepareDataPoint(shared_ptr<Data::MovingHeadDataPoint> dataPoint) {
	auto getResidualFunction = [this](Data::MovingHeadDataPoint * dataPoint) {
		return this->getResidualOnDataPoint(dataPoint);
	};

	dataPoint->getResidualFunction = getResidualFunction;

	auto dataPointWeak = weak_ptr<Data::MovingHeadDataPoint>(dataPoint);

	dataPoint->onTakeCurrent += [dataPointWeak, this]() {
		auto dataPoint = dataPointWeak.lock();
		dataPoint->panTiltSignal.set(this->currentPanTiltSignal.get());
	};

	dataPoint->onGoValue += [dataPointWeak, this]() {
		auto dataPoint = dataPointWeak.lock();
		this->currentPanTiltSignal.set(dataPoint->panTiltSignal.get());
	};

	dataPoint->onGoPrediction += [dataPointWeak, this]() {
		auto dataPoint = dataPointWeak.lock();
		this->navigateToWorldTarget(dataPoint->targetPoint.get());
	};

	dataPoint->onRequestFocus += [dataPointWeak, this]() {
		auto dataPoint = dataPointWeak.lock();
		this->focusedDataPoint = dataPoint;
	};

	dataPoint->isFocused = [dataPoint, this]() {
		return dataPoint == this->focusedDataPoint.lock();
	};
}
//---------
float MovingHead::getResidualOnDataPoint(Data::MovingHeadDataPoint * dataPoint) const {
	auto transform = this->getTransform();

	//--
	//World -> Object space
	//

	//apply rigid body transform
	auto targetInViewSpace4 = glm::inverse(transform) * glm::vec4(dataPoint->targetPoint.get(), 1.0);
	targetInViewSpace4 /= targetInViewSpace4.w;
	auto targetInViewSpace = glm::vec3(targetInViewSpace4);

	if (targetInViewSpace == glm::vec3(0, 0, 0)) {
		//residual is 0 if dataPoint is coincident with fixture
		return 0.0f;
	}
	//
	//--

	// get ideal angles for target points
	auto idealAnglesForTarget = ofxCeres::VectorMath::getPanTiltToTargetInObjectSpace(targetInViewSpace, this->calibrationParameters.tiltOffset.get());

	auto dataPointPanTiltIdeal = this->panTiltSignalToIdeal(dataPoint->panTiltSignal);

	auto disparity = ofxCeres::VectorMath::sphericalPolarDistance(idealAnglesForTarget, dataPointPanTiltIdeal);

	return disparity * RAD_TO_DEG;
}

//----------
void MovingHead::focusDataPointWithHighestResidual() {
	float highestResidual = 0.0f;
	auto dataPoints = this->calibrationPoints.getSelection();

	for (auto dataPoint : dataPoints) {
		auto residual = this->getResidualOnDataPoint(dataPoint.get());
		if (residual > highestResidual) {
			this->focusedDataPoint = dataPoint;
			highestResidual = residual;
		}
	}
}