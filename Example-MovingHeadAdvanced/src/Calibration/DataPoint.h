#pragma once

#include "Data/AbstractCalibrationPoint.h"

namespace Calibration {
	class DataPoint : public Data::AbstractCalibrationPoint {
	public:
		DataPoint();
		string getTypeName() const override;

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);

		int getQuadrant() const;

		ofParameter<glm::vec2> panTiltSignal{ "Pan tilt angles", glm::vec2(0, 0) };
		ofParameter<string> marker{ "Marker", "" };

		float residual = 0.0f;
		float normalisedResidual = 0.0f;

		ofxLiquidEvent<void> onTakeCurrent;
		ofxLiquidEvent<void> onGoValue;
		ofxLiquidEvent<void> onGoPrediction;
	protected:
		ofxCvGui::ElementPtr getDataDisplay() override;
		void overlayMainDisplay(ofxCvGui::ElementPtr) override;
	};
}