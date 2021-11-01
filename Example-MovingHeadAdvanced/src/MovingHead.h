#pragma once

#include "Data/CalibrationPointSet.h"
#include "Data/MovingHeadDataPoint.h"
#include "Markers.h"

MAKE_ENUM(SolveType
	, (Basic, Distorted, Group)
	, ("Basic", "Distorted", "Group"));

class MovingHead : public Data::Serializable, public ofxCvGui::IInspectable {
public:
	MovingHead(shared_ptr<Markers>);
	string getTypeName() const;

	void update();

	void drawWorld(bool selected);

	void serialize(nlohmann::json &);
	void deserialize(const nlohmann::json &, bool isImport = false);
	void populateInspector(ofxCvGui::InspectArguments&);

	shared_ptr<ofxCvGui::Panels::Widgets> getListPanel();

	void solve();
	void solveBasic();
	void solveDistorted();
	void solveGroup();

	void getCalibrationData(vector<glm::vec3>& targetPoints, vector<glm::vec2>& panTiltSignal) const;

	void addTestData();

	glm::mat4 getTransform() const;
	glm::vec3 getPosition() const;
	void applyRotation(const glm::quat&);

	glm::vec2 getPanTiltForWorldTarget(const glm::vec3 &
		, const glm::vec2 & currentPanTiltSignal) const;
	void navigateToWorldTarget(const glm::vec3 &);

	void renderDMX(vector<uint8_t> & dmxValues) const;

	glm::vec2 panTiltIdealToSignal(const glm::vec2 &) const;
	glm::vec2 panTiltSignalToIdeal(const glm::vec2 &) const;

	shared_ptr<Data::CalibrationPointSet<Data::MovingHeadDataPoint>> getCalibrationPoints();
protected:
	void prepareDataPoint(shared_ptr<Data::MovingHeadDataPoint>);
	float getResidualOnDataPoint(Data::MovingHeadDataPoint *) const;
	void focusDataPointWithHighestResidual();

	shared_ptr<Data::CalibrationPointSet<Data::MovingHeadDataPoint>> calibrationPoints = make_shared< Data::CalibrationPointSet<Data::MovingHeadDataPoint>>();

	struct {
		ofParameter<SolveType> solveType{ "Solve type", SolveType::Basic };
	} solveParameters;

	struct {
		ofParameter<float> focus{ "Focus", 0.5f, 0.0f, 1.0f };
	} beamParameters;

	struct {
		ofParameter<glm::vec3> translation{ "Translation", glm::vec3(2.08, 0.78, 4.24), glm::vec3(-10), glm::vec3(+10) };
		ofParameter<glm::vec3> rotationVector{ "Rotation vector", glm::vec3(0, -PI / 2, 0), glm::vec3(-PI / 2), glm::vec3(+PI / 2) };
		ofParameter<glm::vec3> panDistortion{ "Pan distortion", {0, 1, 0} };
		ofParameter<glm::vec3> tiltDistortion{ "Tilt distortion", {0, 1, 0} };
		ofParameter<float> residual{ "Residual", 0 };
	} calibrationParameters;

	struct {
		ofParameter<glm::vec2> panRange{ "Pan range", glm::vec2(-270, +270) };
		ofParameter<glm::vec2> tiltRange{ "Tilt range", glm::vec2(-130, +130) };
		ofParameter<int> dmxPanPolarity{ "DMX pan polarity", 0 };

		struct {
			ofParameter<uint16_t> dmxStartAddress{ "DMX Start Address", 1 };
			ofParameter<uint16_t> panCoarse{ "Pan coarse", 1 };
			ofParameter<uint16_t> panFine{ "Pan fine", 2 };
			ofParameter<uint16_t> tiltCoarse{ "Tilt coarse", 3 };
			ofParameter<uint16_t> tiltFine{ "Tilt fine", 4 };
			ofParameter<uint16_t> brightness{ "Brightness", 5 };
			ofParameter<uint16_t> focusCoarse{ "Focus coarse", 6 };
			ofParameter<uint16_t> focusFine{ "Focus fine", 7 };
		} dmxAddresses;

	} fixtureSettings;

	ofParameter<glm::vec2> currentPanTiltSignal{ "Current Pan Tilt", glm::vec2(0, 0) };
	weak_ptr<Data::MovingHeadDataPoint> focusedDataPoint;

	glm::vec3 lastWorldPosition;

	// Reference to the database of markers so that we can use locally
	shared_ptr<Markers> markers;
};
