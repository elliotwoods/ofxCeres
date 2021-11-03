#pragma once

#include "Data/CalibrationPointSet.h"
#include "ofxCvGui.h"

class Marker : public Data::AbstractCalibrationPoint {
public:
	MAKE_ENUM(Constraint
		, (Free, Origin, Fixed, Plane)
		, ("Free", "Origin", "Fixed", "Plane")
	);

	Marker();
	string getTypeName() const override;
	static string getGlyphForConstraint(const Constraint&);

	ofParameter<glm::vec3> position{ "Position", {0, 0, 0} };
	ofParameter<string> name{ "Name", "" };
	ofParameter<Constraint> constraint{ "Constraint", Constraint::Free };
protected:
	ofxCvGui::ElementPtr getDataDisplay() override;
	static map<Marker::Constraint::Options, string> glyphs;
};
