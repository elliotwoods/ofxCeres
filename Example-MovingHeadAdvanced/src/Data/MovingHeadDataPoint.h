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
		ofParameter<glm::vec2> panTiltSignal{ "Pan tilt angles", glm::vec2(0, 0) };
		ofParameter<glm::vec3> targetPoint{ "Target point", glm::vec3() };

		function<float(MovingHeadDataPoint*)> getResidualFunction;

		ofxLiquidEvent<void> onTakeCurrent;
		ofxLiquidEvent<void> onGoValue;
		ofxLiquidEvent<void> onGoPrediction;
		ofxLiquidEvent<void> onRequestFocus;

		function<bool()> isFocused;
	protected:
		ofxCvGui::ElementPtr getDataDisplay() override;
		void overlayMainDisplay(ofxCvGui::ElementPtr) override;
	};
}