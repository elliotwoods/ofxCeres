#include "pch_ofApp.h"

#include "Controller.h"
#include "../Scene.h"

namespace VR {
	//---------
	Controller::Controller()
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;

		this->openVR.setup(std::bind(&Controller::renderVR
			, this
			, std::placeholders::_1));
		this->openVR.setDrawControllers(true);
		ofAddListener(this->openVR.ofxOpenVRControllerEvent
			, this
			, &Controller::controllerEvent);
	}

	//---------
	Controller::~Controller()
	{
		this->openVR.exit();
	}

	//---------
	string
		Controller::getTypeName() const
	{
		return "VR::Controller";
	}

	//---------
	void
		Controller::update()
	{
		this->openVR.update();

		// Update controllers
		for (int i = 0; i < 2; i++) {
			auto controllerRole = (vr::ETrackedControllerRole)(i + 1);

			auto& controllerState = this->controllerStates[i];

			// Pose
			{
				auto priorPose = controllerState.pose.current;
				auto newPose = this->getControllerPose(controllerRole);
				controllerState.pose.frameRelative = glm::inverse(priorPose) * newPose;
				controllerState.pose.current = newPose;
			}

			if (this->parameters.controllers.calibrateEnabled) {
				// Trackpads
				if (controllerState.trackPad.isDown) {
					// Apply the movement
					{

						auto newPosition = this->openVR.getTrackpadPosition(controllerRole);
						controllerState.trackPad.movement = newPosition - controllerState.trackPad.position;
						controllerState.trackPad.position = newPosition;

						auto movementAmount = glm::length(controllerState.trackPad.movement);
						if (movementAmount > 0 && movementAmount < 0.2f) { // sometimes we get a jump
							Scene::X()->soloMovePanTilt(controllerState.trackPad.movement
								* this->parameters.controllers.trackPad.panTiltMovement.get()
								* (
									this->parameters.controllers.trackPad.delicateCenter
									? glm::length(controllerState.trackPad.position)
									: 1.0f
									)
							);
						}
					}

					// Apply the relative pose (focus control)
					{
						auto frameRotation = glm::quat(controllerState.pose.frameRelative);
						auto frameRotationVector = glm::eulerAngles(frameRotation);
						Scene::X()->soloMoveFocus(frameRotationVector[2]
							* this->parameters.controllers.focusSensitivity.get());
					}
				}
			}

			// If trigger down - pull the lights
			if (this->controllerStates[i].trigger.isDown) {
				Scene::X()->getGroupControl()->navigateTo(this->getControllerPosition(controllerRole));
			}
		}

		// Marker proximity preview
		if (this->parameters.markerProximityPreview.enabled
			&& this->parameters.controllers.calibrateEnabled) {
			bool markerIsCloseToCursor = false;
			auto markers = Scene::X()->getMarkers()->getSelection();
			const auto& distanceThreshold = this->parameters.markerProximityPreview.distanceThreshold.get();

			// For all controllers
			for (int i = 0; i < 2; i++) {
				auto controllerPosition = this->getControllerPosition(i == 0
					? vr::ETrackedControllerRole::TrackedControllerRole_LeftHand
					: vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

				for (auto marker : markers) {
					auto distance = glm::distance(marker->position.get()
						, controllerPosition);
					if (distance < distanceThreshold) {
						markerIsCloseToCursor = true;
						break;
					}
				}
				if (markerIsCloseToCursor) {
					break;
				}
			}

			// Apply the color to all moving heads
			const auto & movingHeads = Scene::X()->getMovingHeads();
			for (auto it : movingHeads) {
				it.second->channels[this->parameters.markerProximityPreview.colorChannel.get()]->setValue(markerIsCloseToCursor
					? this->parameters.markerProximityPreview.colorValue.get()
					: 0);
			}
		}

		// Draw to VR
		if (this->parameters.headset.enabled) {
			this->openVR.render();
			this->openVR.renderDistortion();
		}
	}

	//---------
	void
		Controller::drawWorld()
	{
		this->drawController(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
		this->drawController(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

		if (this->parameters.headset.draw) {
			ofPushStyle();
			{
				ofNoFill();
				ofPushMatrix();
				{
					ofMultMatrix(glm::inverse(this->openVR.getCurrentViewProjectionMatrix(vr::Hmd_Eye::Eye_Left)));
					ofDrawBox(2.0f);
				}
				ofPopMatrix();

				ofPushMatrix();
				{
					ofMultMatrix(glm::inverse(this->openVR.getCurrentViewProjectionMatrix(vr::Hmd_Eye::Eye_Right)));
					ofDrawBox(2.0f);
				}
				ofPopMatrix();
			}
			ofPopStyle();
		}
	}

	//---------
	void 
		Controller::drawController(vr::ETrackedControllerRole controller)
	{
		auto pose = this->getControllerPose(controller);

		ofPushMatrix();
		{
			ofMultMatrix(pose);
			ofDrawAxis(0.3f);
		}
		ofPopMatrix();

	}

	//---------
	glm::mat4
		Controller::getControllerPose(vr::ETrackedControllerRole controller)
	{
		return this->openVR.getControllerPose(controller);
	}

	//---------
	glm::vec3
		Controller::getControllerPosition(vr::ETrackedControllerRole controller)
	{
		if (this->parameters.aimOffset.enabled) {
			glm::vec3 aimOffset(
				this->parameters.aimOffset.x.get()
				, this->parameters.aimOffset.y.get()
				, this->parameters.aimOffset.z.get()
			);
			return ofxCeres::VectorMath::applyTransform(this->getControllerPose(controller)
				, aimOffset);
		}
		else {
			return ofxCeres::VectorMath::applyTransform(this->getControllerPose(controller)
				, glm::vec3(0, 0, 0));
		}
	}

	//---------
	void
		Controller::serialize(nlohmann::json& json)
	{
		Data::serialize(json, this->parameters);
	}

	//---------
	void
		Controller::deserialize(const nlohmann::json& json)
	{
		Data::deserialize(json, this->parameters);
	}

	//---------
	void
		Controller::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
		inspector->addLiveValue<glm::vec3>("Left controller position", [this]() {
			return this->getControllerPosition(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
			});
		inspector->addLiveValue<glm::vec3>("Right controller position", [this]() {
			return this->getControllerPosition(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
			});
		inspector->addParameterGroup(this->parameters);
	}

	//---------
	void
		Controller::renderVR(vr::Hmd_Eye nEye)
	{		
		ofSetMatrixMode(ofMatrixMode::OF_MATRIX_PROJECTION);
		ofPushMatrix();
		{
			ofLoadMatrix(this->openVR.getCurrentProjectionMatrix(nEye));

			ofSetMatrixMode(ofMatrixMode::OF_MATRIX_MODELVIEW);
			ofPushMatrix();
			{
				ofLoadIdentityMatrix();
				ofScale(1, -1, 1);
				ofMultMatrix(this->openVR.getCurrentViewMatrix(nEye));
				auto scene = Scene::X();
				scene->getPanel()->drawGridFromOther();
				scene->drawWorld();
			}
			ofPopMatrix();
		}
		ofSetMatrixMode(ofMatrixMode::OF_MATRIX_PROJECTION);
		ofPopMatrix();
		ofSetMatrixMode(ofMatrixMode::OF_MATRIX_MODELVIEW);
	}

	//---------
	void
		Controller::controllerEvent(ofxOpenVRControllerEventArgs& args)
	{
		auto pose = this->getControllerPose(args.controllerRole);
		auto position = this->getControllerPosition(args.controllerRole);

		auto scene = Scene::X();

		auto controllerIndex = (int)args.controllerRole - 1;

		switch(args.buttonType) {
		case ButtonType::ButtonTrigger:
		{
			auto& triggerState= this->controllerStates[controllerIndex].trigger;

			switch (args.eventType) {
			case EventType::ButtonPress:
			{
				scene->getGroupControl()->navigateTo(position);
				triggerState.isDown = true;
				break;
			}
			case EventType::ButtonUnpress:
			{
				triggerState.isDown = false;
			}
			default:
				break;
			}

			break;
		}
		case ButtonType::ButtonApplicationMenu:
		{
			if (args.eventType == EventType::ButtonPress
				&& this->parameters.controllers.calibrateEnabled) {
				scene->soloNextFixture();
			}
			break;
		}
		case ButtonType::ButtonSystem:
		{
			// Actually this button is weird - let's ignore it
			break;
		}
		case ButtonType::ButtonTouchpad:
		{
			if (this->parameters.controllers.calibrateEnabled) {
				auto& trackpadState = this->controllerStates[controllerIndex].trackPad;

				switch (args.eventType) {
				case EventType::ButtonPress:
					// Store data point
					scene->soloStoreDataPoint(this->getControllerPosition(args.controllerRole));
					break;

				case EventType::ButtonTouch:
					trackpadState.position = {
						args.analogInput_xAxis
						, args.analogInput_yAxis
					};
					trackpadState.movement = { 0, 0 };
					trackpadState.isDown = true;
					break;

				case EventType::ButtonUntouch:
					trackpadState.isDown = false;
					break;
				}
			}

			break;
		}
		case ButtonType::ButtonGrip:
		{
			if (args.eventType == EventType::ButtonPress
				&& this->parameters.controllers.calibrateEnabled) {
				scene->soloCalibrate();
			}
			break;
		}
		default:
			break;
		}
	}
}