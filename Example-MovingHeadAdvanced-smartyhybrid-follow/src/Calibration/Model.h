#pragma once

#include "Data/Serializable.h"
#include "ofxCvGui.h"
#include "ofParameter.h"

namespace DMX {
	class MovingHead;
}

#define MOVINGHEAD_FOCUS_ORDER 3

namespace Calibration {
	class Model : public ofxCvGui::IInspectable, public Data::Serializable {
	public:
		Model(DMX::MovingHead&);
		std::string getTypeName() const override;

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);

		glm::mat4 getTransform() const;
		glm::vec3 getPosition() const;
		void applyRotation(const glm::quat&);

		glm::vec2 getPanTiltForWorldTarget(const glm::vec3&
			, const glm::vec2& currentPanTiltSignal
			, const ofRectangle& boundsLimit = ofRectangle()) const;

		glm::vec2 panTiltIdealToSignal(const glm::vec2&) const;
		glm::vec2 panTiltSignalToIdeal(const glm::vec2&) const;

		ofxCeres::Models::DistortedMovingHead::Solution getDistortedMovingHeadSolution() const;
		void setDistortedMovingHeadSolution(const ofxCeres::Models::DistortedMovingHead::Solution&);

		struct Parameters : ofParameterGroup {
			Parameters();
			ofParameter<glm::vec3> translation{ "Translation", {0, 0, 0} };
			ofParameter<glm::vec3> rotationVector{ "Rotation vector", {0, 0, 0} };
			ofParameter<glm::vec3> panDistortion{ "Pan distortion", {0, 1, 0} };
			ofParameter<glm::vec3> tiltDistortion{ "Tilt distortion", {0, 1, 0} };
			ofParameter<float> residual{ "Residual", 0 };
		} parameters;

		ofxCeres::Models::PolyFit<MOVINGHEAD_FOCUS_ORDER>::Solution focusModel;

	protected:
		DMX::MovingHead& movingHead;
	};
}