#pragma once

#include "Data/CalibrationPointSet.h"
#include "Data/MovingHeadDataPoint.h"

class MovingHead : public Data::Serializable {
public:
	MovingHead();
	string getTypeName() const;

	void update();

	void drawWorld(bool selected);

	void serialize(nlohmann::json &);
	void deserialize(const nlohmann::json &);
	void populateWidgets(shared_ptr<ofxCvGui::Panels::Widgets> widgets);

	void solve();
	void addTestData();

	glm::mat4 getTransform() const;
    
    glm::vec2 getPanTiltForWorldTarget(const glm::vec3 &
		, const glm::vec2 & currentPanTilt) const;
	void navigateToWorldTarget(const glm::vec3 &);

	void setWorldCursorPosition(const glm::vec3 &);
protected:
	void prepareDataPoint(shared_ptr<Data::MovingHeadDataPoint>);
	float getResidualOnDataPoint(Data::MovingHeadDataPoint *) const;

	Data::CalibrationPointSet<Data::MovingHeadDataPoint> calibrationPoints;
	ofParameter<glm::vec3> translation{ "Translation", glm::vec3(2.08, 0.78, 4.24), glm::vec3(-10), glm::vec3(+10) };
	ofParameter<glm::vec3> rotationVector{ "Rotation vector", glm::vec3(0, -PI / 2, 0), glm::vec3(-PI / 2), glm::vec3(+PI / 2) };
	ofParameter<float> tiltOffset{ "Tilt offset", 0, -20, 20 };

	struct {
		ofParameter<glm::vec2> panRange{ "Pan range", glm::vec2(-270, +270) };
		ofParameter<glm::vec2> tiltRange{ "Tilt range", glm::vec2(-130, +130) };
		
		struct {
			ofParameter<uint16_t> panCoarse{ "Pan coarse", 0 };
			ofParameter<uint16_t> panFine{ "Pan fine", 1 };
			ofParameter<uint16_t> tiltCoarse{ "Tilt coarse", 2 };
			ofParameter<uint16_t> tiltFine{ "Tilt fine", 3 };
			ofParameter<uint16_t> brightness{ "Brightness", 4 };
		} dmxAddresses;

	} fixtureSettings;

	ofParameter<glm::vec2> currentPanTilt{ "Current Pan Tilt", glm::vec2(0, 0) };
	weak_ptr<Data::MovingHeadDataPoint> focusedDataPoint;
};
