#include "pch_ofApp.h"
#include "MovingHead.h"

//---------
MovingHead::MovingHead() {
	RULR_SERIALIZE_LISTENERS;
}

//---------
string MovingHead::getTypeName() const {
	return "MovingHead";
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

		//draw moving head
		ofNoFill();
		vector<glm::vec3> transmissionsInObjectSpace;
		ofPushMatrix();
		{
			ofMultMatrix(transform);

			//draw hardware
			ofSetColor(selected ? 220 : 100);
			ofDrawAxis(0.4f);
			ofSetSphereResolution(6);
			ofDrawBox(glm::vec3(0, -0.35, 0), 0.5, 0.1, 0.4);
			ofPushMatrix();
			{
				ofScale(0.6, 1, 0.6);
				ofDrawSphere(0.2f);
			}
			ofPopMatrix();
		}
		ofPopMatrix();

		//draw selected data point
		auto selectedDataPoint = this->selectedDataPoint.lock();
		if (selectedDataPoint) {
			ofPushStyle();
			{
				ofNoFill();
				ofDrawCircle(selectedDataPoint->targetPoint.get(), 0.05f);
			}
			ofPopStyle();
		}

		//draw rays and residuals
		if (selected) {
			ofPushStyle();
			{
				for (const auto & calibrationPoint : calibrationPoints) {
					ofSetColor(calibrationPoint->color);
					auto transmissionObject = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt(calibrationPoint->getPanTiltAngles()
						, this->tiltOffset.get());

					auto transmissionWorld4 = transform * glm::vec4(transmissionObject, 1.0f);
					auto transmissionWorld = (glm::vec3) (transmissionWorld4 / transmissionWorld4.w);

					ofSetLineWidth(2.0f);
					ofDrawLine(this->translation.get(), transmissionWorld);
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

	json << this->translation;
	json << this->rotationVector;
	json << this->tiltOffset;
}

//---------
void MovingHead::deserialize(const nlohmann::json & json) {
	if (json.count("calibrationPoints") != 0) {
		this->calibrationPoints.deserialize(json["calibrationPoints"]);
		auto calibrationPoints = this->calibrationPoints.getAllCaptures();

		auto getResidualFunction = [this](Data::MovingHeadDataPoint * dataPoint) {
			return this->getResidualOnDataPoint(dataPoint);
		};

		for (auto calibrationPoint : calibrationPoints) {
			calibrationPoint->getResidualFunction = getResidualFunction;
		}
	}

	json >> this->translation;
	json >> this->rotationVector;
	json >> this->tiltOffset;
}

//---------
void MovingHead::populateWidgets(shared_ptr<ofxCvGui::Panels::Widgets> widgets) {
	this->calibrationPoints.populateWidgets(widgets);

	widgets->addButton("Toggle selection", [this]() {
		auto selectedDataPoint = this->selectedDataPoint.lock();
		if (selectedDataPoint) {
			selectedDataPoint->setSelected(!selectedDataPoint->isSelected());
		}
	}, ' ');

	widgets->addButton("Solve", [this]() {
		this->solve();
	}, OF_KEY_RETURN)->setHeight(100.0f);

	widgets->addEditableValue<glm::vec3>(this->translation);
	widgets->addEditableValue<glm::vec3>(this->rotationVector);
	widgets->addSlider(this->tiltOffset);

	widgets->addButton("Add test data", [this]() {
		this->addTestData();
	});
}

//---------
void MovingHead::solve() {
	vector<glm::vec3> targetPoints;
	vector<glm::vec2> panTiltAngles;

	auto calibrationPoints = this->calibrationPoints.getSelection();
	for (auto calibrationPoint : calibrationPoints) {
		targetPoints.push_back(calibrationPoint->targetPoint);
		panTiltAngles.push_back(calibrationPoint->getPanTiltAngles());
	}

	auto result = ofxCeres::Models::MovingHead::solve(targetPoints
		, panTiltAngles
		, ofxCeres::Models::MovingHead::Solution {
		this->translation.get()
			, this->rotationVector.get()
			, this->tiltOffset.get()
	});
	this->translation = result.solution.translation;
	this->rotationVector = result.solution.rotationVector;
	this->tiltOffset = result.solution.tiltOffset;
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

	auto getResidualFunction = [this](Data::MovingHeadDataPoint * dataPoint) {
		return this->getResidualOnDataPoint(dataPoint);
	};

	for (size_t i = 0; i < names.size(); i++) {
		auto newDataPoint = make_shared<Data::MovingHeadDataPoint>();
		newDataPoint->name = names[i];
		newDataPoint->dmxValues = dmxValues[i];
		newDataPoint->targetPoint = targetPoints[i];

		newDataPoint->getResidualFunction = getResidualFunction;
		this->calibrationPoints.add(newDataPoint);
	}
}

//---------
glm::mat4 MovingHead::getTransform() const {
	return ofxCeres::VectorMath::createTransform(this->translation.get()
		, this->rotationVector.get());
}

//---------
glm::vec2 MovingHead::getPanTiltForWorldPosition(const glm::vec3 & world) const {
    auto objectSpacePosition4 = glm::inverse(this->getTransform()) * glm::vec4(world, 1.0f);
    auto objectSpacePosition = (glm::vec3) (objectSpacePosition4 / objectSpacePosition4.w);
    auto panTiltAngles = ofxCeres::VectorMath::getPanTiltToTargetInObjectSpace(objectSpacePosition
                                                                               , this->tiltOffset.get());
    return panTiltAngles;
}

//---------
void MovingHead::setWorldCursorPosition(const glm::vec3 & position) {
	auto minDistance = numeric_limits<float>::max();
	auto dataPoints = this->calibrationPoints.getAllCaptures();
	for (auto dataPoint : dataPoints) {
		auto distance = glm::distance2(dataPoint->targetPoint.get(), position);
		if (distance < minDistance) {
			this->selectedDataPoint = dataPoint;
			minDistance = distance;
		}
	}
}

//---------
float MovingHead::getResidualOnDataPoint(Data::MovingHeadDataPoint * dataPoint) const {
	// copied out of ofxCeres::Models::MovingHead cpp file

	auto transform = ofxCeres::VectorMath::createTransform(translation.get(), rotationVector.get());

	//--
	//World -> Object space
	//

	//apply rigid body transform
	auto targetInViewSpace4 = glm::inverse(transform) * glm::vec4(dataPoint->targetPoint.get(), 1.0);
	targetInViewSpace4 /= targetInViewSpace4.w;
	auto targetInViewSpace = glm::vec3(targetInViewSpace4);

	//
	//--

	auto rayCastForPanTiltValues = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt<float>(dataPoint->getPanTiltAngles(), tiltOffset);

	//--
	//Get the disparity between the real and actual object space rays
	//
	auto dotProduct = ofxCeres::VectorMath::dot(rayCastForPanTiltValues, targetInViewSpace);
	dotProduct = dotProduct / (ofxCeres::VectorMath::length(rayCastForPanTiltValues) * ofxCeres::VectorMath::length(targetInViewSpace));
	auto angleBetweenResults = acos(dotProduct);

	return angleBetweenResults * RAD_TO_DEG;
}
