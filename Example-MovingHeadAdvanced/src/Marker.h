#pragma once

#include "Data/CalibrationPointSet.h"
#include "ofxCvGui.h"

class Marker : public Data::AbstractCalibrationPoint {
public:
	MAKE_ENUM(Constraint
		, (Free, Origin, Fixed, Direction)
		, ("Free", "Origin", "Fixed", "Direction")
	);

	Marker();
	string getTypeName() const override;
	ofParameter<glm::vec3> position{ "Position", {0, 0, 0} };
	ofParameter<string> name{ "Name", "" };
	ofParameter<Constraint> constraint{ "Constraint", Constraint::Free };
protected:
	ofxCvGui::ElementPtr getDataDisplay() override;
};
