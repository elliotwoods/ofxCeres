#pragma once

#include "ofxOpenVR.h"
#include "Data/Serializable.h"
#include "ofxCvGui.h"

namespace VR {
	class Controller : public ofxCvGui::IInspectable, public Data::Serializable {
	public:
		Controller();
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
				PARAM_DECLARE("Heaset", enabled);
			} headset;

			struct : ofParameterGroup {
				struct : ofParameterGroup {
					ofParameter<float> panTiltMovement{ "Pan tilt movement", 20.0, 0, 90 };
					ofParameter<bool> delicateCenter{ "Delicate center", true };
					PARAM_DECLARE("Trackpad", panTiltMovement, delicateCenter);
				} trackPad;
				ofParameter<float> focusSensitivity{ "Focus sensitivity", 0.1, 0, 1 };
				PARAM_DECLARE("Controllers", trackPad, focusSensitivity);
				} controllers;

			PARAM_DECLARE("Controller", headset, controllers);
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