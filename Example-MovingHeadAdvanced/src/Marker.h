#pragma once

#include "Data/CalibrationPointSet.h"
#include "ofxCvGui.h"

class Marker : public Data::AbstractCalibrationPoint {
public:
	Marker();
	string getTypeName() const override;
	ofParameter<glm::vec3> position{ "Position", {0, 0, 0} };
	ofParameter<string> name{ "Name", "" };
protected:
	ofxCvGui::ElementPtr getDataDisplay() override;
};
