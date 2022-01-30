#pragma once

#include "ofxOpenVR.h"
#include "Data/Serializable.h"
#include "ofxCvGui.h"
#include "../DMX/Types.h"

namespace VR {
	class Controller : public ofxCvGui::IInspectable, public Data::Serializable {
	public:
		Controller();
		~Controller();

		string getTypeName() const override;

		void update();

		void drawWorld();
		void drawController(vr::ETrackedControllerRole);

		glm::mat4 getControllerPose(vr::ETrackedControllerRole);
		glm::vec3 getControllerPosition(vr::ETrackedControllerRole);

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);
	protected:
		void renderVR(vr::Hmd_Eye nEye);
		void controllerEvent(ofxOpenVRControllerEventArgs& args);
		
		struct : ofParameterGroup {
			struct : ofParameterGroup {
				ofParameter<bool> enabled { "Enabled", false };
				ofParameter<bool> draw{ "Draw",true };
				PARAM_DECLARE("Heaset", enabled, draw);
			} headset;

			struct : ofParameterGroup {
				ofParameter<bool> calibrateEnabled{ "Calibrate enabled", true };

				struct : ofParameterGroup {
					ofParameter<float> panTiltMovement{ "Pan tilt movement", 20.0, 0, 90 };
					ofParameter<bool> delicateCenter{ "Delicate center", true };
					PARAM_DECLARE("Trackpad", panTiltMovement, delicateCenter);
				} trackPad;
				ofParameter<float> focusSensitivity{ "Focus sensitivity", 0.1, 0, 1 };
				PARAM_DECLARE("Controllers", calibrateEnabled, trackPad, focusSensitivity);
				} controllers;

			struct : ofParameterGroup {
				ofParameter<bool> enabled{ "Enabled", false };
				ofParameter<float> distanceThreshold{ "Distance threshold", 0.1f };
				ofParameter<DMX::ChannelIndex> colorChannel{ "Color channel", 0 };
				ofParameter<DMX::Value> colorValue{ "Color value", 38 };
				PARAM_DECLARE("Marker proximity preview", enabled, distanceThreshold, colorChannel, colorValue);
			} markerProximityPreview;

			struct : ofParameterGroup {
				ofParameter<bool> enabled{ "Enabled", false };
				ofParameter<float> x{ "X", 0, -0.2, 0.2 };
				ofParameter<float> y{ "Y", 0, -0.2, 0.2 };
				ofParameter<float> z{ "Z", 0, -0.2, 0.2 };
				PARAM_DECLARE("Aim offset", enabled, x, y, z);
			} aimOffset;
			PARAM_DECLARE("Controller", headset, controllers, markerProximityPreview, aimOffset);
		} parameters;

		ofxOpenVR openVR;

		struct ControllerState {
			struct Trackpad {
				bool isDown = false;
				glm::vec2 position;
				glm::vec2 movement;
			} trackPad;

			struct Trigger {
				bool isDown = false;
			} trigger;

			struct Pose {
				glm::mat4 current;
				glm::mat4 frameRelative;
			} pose;
		};

		ControllerState controllerStates[2];
	};
}