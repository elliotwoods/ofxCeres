#include "pch_ofApp.h"
#include "MovingHead.h"
#include "Widgets/PanTiltTrackpad.h"
#include "Scene.h"

namespace DMX {
	//----------
	MovingHead::MovingHead(const Configuration& configuration)
	{
		// Apply configuration
		{
			this->parameters.pan.setMin(configuration.minimumPan);
			this->parameters.pan.setMax(configuration.maximumPan);
			this->parameters.tilt.setMin(configuration.minimumTilt);
			this->parameters.tilt.setMax(configuration.maximumTilt);
		}

		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;

		// OSC routers
		{
			this->staticRoute("navigateTo", [this](const ofxOscMessage& message) {
				if (message.getNumArgs() >= 3) {
					this->navigateToWorldTarget(glm::vec3(message.getArgAsFloat(0)
						, message.getArgAsFloat(1)
						, message.getArgAsFloat(1)
					));
				}
				});

			this->dynamicRoute([this](const OSC::Path& path, const ofxOscMessage& message) {
				// Handle parameters
				if (message.getNumArgs() > 0) {

					for (auto& parameter : this->parameters) {
						if (OSC::Path::stripName(parameter->getName()) == path[0]) {
							{
								auto typedParameter = dynamic_pointer_cast<ofParameter<float>>(parameter);
								if (typedParameter) {
									typedParameter->set(message.getArgAsFloat(0));
									return true;
								}
							}
							{
								auto typedParameter = dynamic_pointer_cast<ofParameter<int>>(parameter);
								if (typedParameter) {
									typedParameter->set(message.getArgAsInt(0));
									return true;
								}
							}
							{
								auto typedParameter = dynamic_pointer_cast<ofParameter<bool>>(parameter);
								if (typedParameter) {
									typedParameter->set(message.getArgAsInt(0) != 0);
									return true;
								}
							}
						}
					}
				}
				return false;
				});
		}
	}

	//----------
	void
	MovingHead::update()
	{
		Fixture::update();
		this->solver->update();
	}

	//----------
	void
	MovingHead::drawWorld()
	{
		ofPushStyle();
		{
			auto transform = this->model->getTransform();

			//draw moving head body
			ofNoFill();
			vector<glm::vec3> transmissionsInObjectSpace;
			ofPushMatrix();
			{
				ofMultMatrix(transform);

				//draw hardware
				{
					ofSetColor(this->isBeingInspected() ? 220 : 100);
					ofDrawAxis(0.4f);

					// base
					ofDrawBox(glm::vec3(0, -0.35, 0), 0.5, 0.1, 0.4);

					auto panTiltIdeal = this->model->panTiltSignalToIdeal(this->getCurrentPanTilt());

					// arrow
					static ofMesh arrow;
					if (arrow.getNumVertices() == 0) {
						arrow.addVertices(
							{
								{ -0.05, -0.3, 0.3 },
								{ +0.05, -0.3, 0.3 },
								{ 0, -0.3, 0.4 }
							});
					}
					arrow.drawWireframe();

					// Axis 1
					ofPushMatrix();
					{
						ofRotateDeg(-panTiltIdeal.x, 0, 1, 0);

						ofDrawBox({ -0.15, -0.3 + 0.35f / 2.0f, 0 }, 0.075, 0.35, 0.2);
						ofDrawBox({ +0.15, -0.3 + 0.35f / 2.0f, 0 }, 0.075, 0.35, 0.2);

						// Axis 2
						ofPushMatrix();
						{
							ofRotateDeg(panTiltIdeal.y, 1, 0, 0);

							// head
							ofPushMatrix();
							{
								ofSetSphereResolution(6);
								ofScale(0.1, 0.2, 0.1);
								ofDrawSphere(1.0);
							}
							ofPopMatrix();

							// beam
							ofPushMatrix();
							{
								ofSetColor(this->isBeingInspected() ? 255 : 100);
								if (this->parameters.shutter.get()) {
									ofTranslate(0.0f, 0.05 + 0.5f / 2.0f, 0.0f);
									ofDrawCylinder(0.02, 0.5f);
								}
								else {
									ofDrawLine(glm::vec3(0, 0, 0)
										, glm::vec3(0.f, 0.5f, 0.f));
								}
							}
							ofPopMatrix();
						}
						ofPopMatrix();
					}
					ofPopMatrix();
				}
			}
			ofPopMatrix();
		}
		ofPopStyle();

		this->solver->drawWorld();
	}

	//----------
	void
	MovingHead::serialize(nlohmann::json& json)
	{
		Data::serialize(json, this->parameters);

		this->model->notifySerialize(json["model"]);
		this->solver->notifySerialize(json["solver"]);
	}

	//----------
	void
	MovingHead::deserialize(const nlohmann::json& json)
	{
		Data::deserialize(json, this->parameters);

		if (json.contains("model")) {
			this->model->notifyDeserialize(json["model"]);
		}
		if (json.contains("solver")) {
			this->solver->notifyDeserialize(json["solver"]);
		}
	}

	//----------
	void
	MovingHead::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
		inspector->addParameterGroup(this->parameters);
		{
			auto trackpad = make_shared<Widgets::PanTiltTrackpad>(this->parameters.pan, this->parameters.tilt);
			auto trackpadWeak = weak_ptr<Widgets::PanTiltTrackpad>(trackpad);
			trackpad->onDraw += [this, trackpadWeak](ofxCvGui::DrawArguments& args) {
				auto trackpad = trackpadWeak.lock();
				const auto& boundsLimit = this->parameters.boundsLimit.get();
				if (boundsLimit.getArea() != 0.0f) {
					ofRectangle boundsInDraw;
					auto topLeft = trackpad->toXY(boundsLimit.getTopLeft());
					auto bottomRight = trackpad->toXY(boundsLimit.getBottomRight());
					ofPushStyle();
					{
						ofEnableAlphaBlending();
						ofSetColor(0, 255, 0, 40);
						ofDrawRectangle(topLeft, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
					}
					ofPopStyle();
				}
			};
			inspector->add(trackpad);
		}
		inspector->addButton("Home", [this]() {
			this->parameters.pan.set(0);
			this->parameters.tilt.set(0);
			}, 'h');
		inspector->addButton("Flip", [this]() {
			this->flip();
			}, 'l');
		inspector->addToggle("Solo", [this]() {
			return this->getSolo();
			}, [this](bool solo) {
			this->setSolo(solo);
			})->setHotKey('s');
		inspector->addSubMenu("Model", this->model);
		inspector->addSubMenu("Solver", this->solver);
	}

	//---------
	glm::vec2
	MovingHead::getCurrentPanTilt() const
	{
		return glm::vec2(
			this->parameters.pan.get()
			, this->parameters.tilt.get()
		);
	}

	//---------
	void
	MovingHead::clampPanTilt()
	{
		if (this->parameters.pan.get() < this->parameters.pan.getMin()) {
			this->parameters.pan.set(this->parameters.pan.getMin());
		}
		if (this->parameters.pan.get() > this->parameters.pan.getMax()) {
			this->parameters.pan.set(this->parameters.pan.getMax());
		}
		if (this->parameters.tilt.get() < this->parameters.tilt.getMin()) {
			this->parameters.tilt.set(this->parameters.tilt.getMin());
		}
		if (this->parameters.tilt.get() > this->parameters.tilt.getMax()) {
			this->parameters.tilt.set(this->parameters.tilt.getMax());
		}
	}

	//---------
	void
	MovingHead::clampFocus()
	{
		if (this->parameters.focus.get() < this->parameters.focus.getMin()) {
			this->parameters.focus.set(this->parameters.focus.getMin());
		}
		if (this->parameters.focus.get() > this->parameters.focus.getMax()) {
			this->parameters.focus.set(this->parameters.focus.getMax());
		}
	}

	//---------
	void
	MovingHead::navigateToWorldTarget(const glm::vec3& world)
	{
		// Navigate pan-tilt values
		auto panTiltAngles = this->model->getPanTiltForWorldTarget(world
			, this->getCurrentPanTilt()
			, this->parameters.boundsLimit.get());
		this->parameters.pan.set(panTiltAngles.x);
		this->parameters.tilt.set(panTiltAngles.y);
	

		// Navigate the focus
		{
			//check 'Fit focus values.ipynb'
			auto distance = glm::distance(world, this->model->getPosition());

			auto focus = model->focusModel(1.0 / distance);
			if (focus < 0.0f) {
				focus = 0.0f;
			}
			else if (focus > 1.0f) {
				focus = 1.0f;
			}
			this->parameters.focus.set(focus);
		}
	}

	//---------
	shared_ptr<Calibration::Model>
	MovingHead::getModel()
	{
		return this->model;
	}

	//---------
	shared_ptr<Calibration::Solver>
	MovingHead::getSolver()
	{
		return this->solver;
	}

	//---------
	void
	MovingHead::flip()
	{
		auto panTilt = this->model->panTiltSignalToIdeal(this->getCurrentPanTilt());

		// All ideal flips
		vector<glm::vec2> flips = {
			panTilt + glm::vec2(360, 0)
			, panTilt - glm::vec2(360, 0)
			, glm::vec2(panTilt.x + 180, -panTilt.y)
			, glm::vec2(panTilt.x - 180, -panTilt.y)
		};

		// Check which ones are in range
		vector<glm::vec2> possibleFlips;
		for (const auto& flip : flips) {
			auto panTiltSignal = this->model->panTiltIdealToSignal(flip);
			if (panTiltSignal.x >= this->parameters.pan.getMin()
				&& panTiltSignal.x <= this->parameters.pan.getMax()
				&& panTiltSignal.y >= this->parameters.tilt.getMin()
				&& panTiltSignal.y <= this->parameters.tilt.getMax()) {
				// Take the first if this flip is available
				this->parameters.pan.set(panTiltSignal.x);
				this->parameters.tilt.set(panTiltSignal.y);
				return;
			}
		}
	}

	//---------
	void
	MovingHead::toggleSolo()
	{
		this->setSolo(!this->getSolo());
	}

	//---------
	bool
	MovingHead::getSolo() const
	{
		const auto& movingHeads = Scene::X()->getMovingHeads();
		for (const auto& it : movingHeads) {
			if (it.second.get() == this) {
				// ignore this
				continue;
			}
			if (it.second->parameters.shutter.get()) {
				return false;
				break;
			}
		}
		return true;
	}

	//---------
	void
	MovingHead::setSolo(bool solo)
	{
		const auto& movingHeads = Scene::X()->getMovingHeads();

		for (const auto& it : movingHeads) {
			if (it.second.get() == this) {
				// ignore this
				continue;
			}
			it.second->parameters.shutter.set(!solo);
		}

		// anyway turn this one on
		this->parameters.shutter.set(true);
	}
}