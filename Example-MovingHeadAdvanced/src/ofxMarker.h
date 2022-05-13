#pragma once

#include "Data/CalibrationPointSet.h"
#include "ofxCvGui.h"

class ofxMarker : public Data::AbstractCalibrationPoint {
public:
	MAKE_ENUM(Constraint
		, (Free, Fixed, Plane)
		, ("Free", "Fixed", "Plane")
	);

	ofxMarker();
	string getTypeName() const override;

	static string getGlyphForConstraint(const Constraint&);

	void serialize(nlohmann::json&) const;
	void deserialize(const nlohmann::json&);

	ofParameter<glm::vec3> position{ "Position", {0, 0, 0} };
	ofParameter<string> name{ "Name", "" };
	ofParameter<Constraint> constraint{ "Constraint", Constraint::Free };
	
protected:
	ofxCvGui::ElementPtr getDataDisplay() override;

	static map<ofxMarker::Constraint::Options, string> glyphs;
};
