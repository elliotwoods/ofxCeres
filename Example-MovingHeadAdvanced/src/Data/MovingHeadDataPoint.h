#pragma once

#include "AbstractCalibrationPoint.h"

namespace Data {
	class MovingHeadDataPoint : public AbstractCalibrationPoint {
	public:
		MovingHeadDataPoint();
		string getTypeName() const override;
		void drawWorld() const;

		void serialize(nlohmann::json &);
		void deserialize(const nlohmann::json &);

		ofParameter<string> name{ "Name", "" };
		ofParameter<glm::vec4> dmxValues{ "DMX Values", glm::vec4(127, 0, 127, 0) };
		ofParameter<glm::vec3> targetPoint{ "Target point", glm::vec3() };
		glm::vec2 getPanTiltAngles() const; // get the pan-tilt angles in degrees

		function<float(MovingHeadDataPoint*)> getResidualFunction;
	protected:
		ofxCvGui::ElementPtr getDataDisplay() override;
	};
}