#pragma once

#include "Data/CalibrationPointSet.h"
#include "Data/MovingHeadDataPoint.h"

class MovingHead : public Data::Serializable {
public:
	MovingHead();
	string getTypeName() const;

	void drawWorld(bool selected);

	void serialize(nlohmann::json &);
	void deserialize(const nlohmann::json &);
	void populateWidgets(shared_ptr<ofxCvGui::Panels::Widgets> widgets);

	void solve();
	void addTestData();

	glm::mat4 getTransform() const;

	void setWorldCursorPosition(const glm::vec3 &);
protected:
	float getResidualOnDataPoint(Data::MovingHeadDataPoint *) const;

	Data::CalibrationPointSet<Data::MovingHeadDataPoint> calibrationPoints;
	ofParameter<glm::vec3> translation{ "Translation", glm::vec3(2.08, 0.78, 4.24), glm::vec3(-10), glm::vec3(+10) };
	ofParameter<glm::vec3> rotationVector{ "Rotation vector", glm::vec3(0, -PI / 2, 0), glm::vec3(-PI / 2), glm::vec3(+PI / 2) };
	ofParameter<float> tiltOffset{ "Tilt offset", 0, -20, 20 };

	weak_ptr<Data::MovingHeadDataPoint> selectedDataPoint;
};