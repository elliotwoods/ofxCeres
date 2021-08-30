#include "pch_ofApp.h"
#include "StewartPlatform.h"
#include "Solvers/StewartPlatformForces.h"
#include "Solvers/StewartPlatformFK.h"
#include <glm/gtx/matrix_decompose.hpp>

template<typename T>
void serialize(nlohmann::json& json, const ofParameter<T> & parameter)
{
	json[parameter.getName()] = parameter.get();
}

template<typename T>
void deserialize(const nlohmann::json& json, ofParameter<T> & parameter)
{
	parameter.set(json[parameter.getName()]);
}

template<>
void serialize(nlohmann::json& json, const ofParameter<glm::vec3>& parameter)
{
	json[parameter.getName()]["x"] = parameter.get().x;
	json[parameter.getName()]["y"] = parameter.get().y;
	json[parameter.getName()]["z"] = parameter.get().z;
}

template<>
void deserialize(const nlohmann::json& json, ofParameter<glm::vec3>& parameter)
{
	if (!json.is_object() || json.is_null()) {
		return;
	}
	glm::vec3 value = parameter.get();
	if (json.contains("x")) {
		value.x = json["x"];
	}
	if (json.contains("y")) {
		value.z = json["y"];
	}
	if (json.contains("z")) {
		value.z = json["z"];
	}

	parameter.set(value);
}

ofLight light;
ofMaterial material;

namespace Data
{
	//----------
	StewartPlatform::Actuators::Actuator::Actuator()
	{
		this->joints["lower"] = SA::System::Joint();
		this->joints["upper"] = SA::System::Joint();

		this->onDraw += [this]()
		{
			ofDrawLine(this->joints["lower"].position, this->joints["upper"].position);
		};
	}

	//----------
	StewartPlatform::Deck::Deck()
	{
		auto changeCallback = [this](float&) {
			this->onChange.notifyListeners();
			this->markDirty();
		};
		this->diameterChangeListener = this->diameter.newListener(changeCallback);
		this->jointSpacingChangeListener = this->jointSpacing.newListener(changeCallback);
		this->rotationOffsetChangeListener = this->rotationOffset.newListener(changeCallback);
		this->rotationXChangeListener = this->rotationX.newListener(changeCallback);
		this->onDraw += [this]() {
			this->line.draw();

			ofPushMatrix();
			{
				ofScale(0.8f, 0.8f, 0.8f);
				this->line.draw();
			}
			ofPopMatrix();
		};
		this->markDirty();
	}

	//----------
	void
		StewartPlatform::Deck::update()
	{
		if (this->isDirty)
		{
			this->rebuild();
		}
	}

	//----------
	void
		StewartPlatform::Deck::markDirty()
	{
		this->isDirty = true;
	}

	//----------
	void
		StewartPlatform::Deck::serialize(nlohmann::json& json)
	{
		::serialize(json, this->diameter);
		::serialize(json, this->jointSpacing);
		::serialize(json, this->rotationOffset);
		::serialize(json, this->rotationX);
	}

	//----------
	void
		StewartPlatform::Deck::deserialize(const nlohmann::json& json)
	{
		::deserialize(json, this->diameter);
		::deserialize(json, this->jointSpacing);
		::deserialize(json, this->rotationOffset);
		::deserialize(json, this->rotationX);
	}

	//----------
	void
		StewartPlatform::solveForces()
	{
		auto solverSettings = this->getDefaultSolverSettings();
		auto result = Solvers::StewartPlatformForces::solve(*this, true, solverSettings);
		this->forcesSolved = result.isConverged();
		if (result.isConverged()) {
			this->needsForceSolve = false;
		}
	}

	//----------
	void
		StewartPlatform::solveIK()
	{
		for (int i = 0; i < 6; i++)
		{
			auto actuator = this->actuators.actuators[i];
			auto upperPosition = actuator->getJointPosition("upper");
			auto lowerPosition = actuator->getJointPosition("lower");

			//Update the length
			actuator->value.set(glm::distance(upperPosition, lowerPosition));
		}
	}

	//----------
	void
		StewartPlatform::solveFK()
	{
		auto solverSettings = this->getDefaultSolverSettings();
		auto result = Solvers::StewartPlatformFK::solve(*this, true, solverSettings);
		this->fkSolved = result.isConverged();
		if (result.isConverged()) {
			this->needsFKSolve = false;
		}

		// Rebuild but don't IK
		this->rebuild(false);
	}

	//----------
	bool
		StewartPlatform::isForcesSolved() const
	{
		return this->forcesSolved;
	}

	//----------
	bool
		StewartPlatform::isFKSolved() const
	{
		return this->fkSolved;
	}

	//----------
	bool
		StewartPlatform::isValidTransform(const glm::vec3& translation, const glm::vec3& rotation)
	{
		this->transform.translate.x.set(translation.x);
		this->transform.translate.y.set(translation.y);
		this->transform.translate.z.set(translation.z);

		this->transform.rotate.x.set(rotation.x);
		this->transform.rotate.y.set(rotation.y);
		this->transform.rotate.z.set(rotation.z);

		this->transformFromParameters();
		this->rebuild(false);
		this->solveIK();

		for (int i = 0; i < 6; i++) {
			const auto& value = this->actuators.actuators[i]->value;
			if (value.get() > value.getMax() || value.get() < value.getMin()) {
				return false;
			}
		}
		return true;
	}

	//----------
	void
		StewartPlatform::transformFromParameters()
	{
		// Update the upper deck transform
		const auto translation = glm::vec3(this->transform.translate.x.get()
			, this->transform.translate.y.get()
			, this->transform.translate.z.get());

		const auto rotationVector = glm::vec3(this->transform.rotate.x.get() * DEG_TO_RAD
			, this->transform.rotate.y.get() * DEG_TO_RAD
			, this->transform.rotate.z.get() * DEG_TO_RAD);

		const auto orientation = ofxCeres::VectorMath::eulerToQuat(rotationVector);

		this->upperDeck->setOrientation(orientation);
		this->upperDeck->setPosition(translation);

		this->needsRebuild = true;
		this->needsTransformFromParameters = false;
	}

	//----------
	void
		StewartPlatform::transformToParameters()
	{
		auto transform = this->upperDeck->getLocalTransformMatrix();

		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform
			, scale
			, orientation
			, translation
			, skew
			, perspective);

		{
			this->transform.translate.x = translation.x;
			this->transform.translate.y = translation.y;
			this->transform.translate.z = translation.z;
		}

		{
			auto eulerAngles = glm::eulerAngles(orientation);
			this->transform.rotate.x = eulerAngles.x * RAD_TO_DEG;
			this->transform.rotate.y = eulerAngles.y * RAD_TO_DEG;
			this->transform.rotate.z = eulerAngles.z * RAD_TO_DEG;
		}

		this->needsTransformFromParameters = false;
		this->needsTransformToParameters = false;
	}

	//----------
	void
		StewartPlatform::Deck::rebuild()
	{
		this->jointAngleOffset = asin(this->jointSpacing.get() / this->diameter.get());

		auto vertex = [this](const float& theta)
		{
			return glm::vec3(
				sin(theta) * this->diameter.get() / 2.0f
				, 0
				, cos(theta) * this->diameter.get() / 2.0f
			);
		};

		auto A = 0;
		auto B = this->rotationOffset * TWO_PI;
		auto C = -this->rotationOffset * TWO_PI;

		this->joints["A1"].position = vertex(A - this->jointAngleOffset);
		this->joints["A2"].position = vertex(A + this->jointAngleOffset);
		this->joints["B1"].position = vertex(B - this->jointAngleOffset);
		this->joints["B2"].position = vertex(B + this->jointAngleOffset);
		this->joints["C1"].position = vertex(C - this->jointAngleOffset);
		this->joints["C2"].position = vertex(C + this->jointAngleOffset);

		// Apply our offset transform
		{
			auto rotation = ofxCeres::VectorMath::eulerToQuat(glm::vec3(this->rotationX.get() * DEG_TO_RAD, 0.0f, 0.0f));
			for (auto& joint : this->joints) {
				joint.second.position = rotation * joint.second.position;
			}
		}

		this->line.clear();
		for (const auto& joint : this->joints) {
			if (joint.first == "Ground") {
				continue;
			}
			this->line.addVertex(joint.second.position);
		}
		this->line.close();

		this->isDirty = false;
	}

	//----------
	ofxCeres::SolverSettings
		StewartPlatform::getDefaultSolverSettings() const
	{
		ofxCeres::SolverSettings solverSettings;
		solverSettings.options.function_tolerance = 0.0;
		solverSettings.options.max_num_iterations = this->solveOptions.maxIterations.get();

		if (this->solveOptions.printOutput) {
			solverSettings.printReport = true;
			solverSettings.options.minimizer_progress_to_stdout = true;
		}
		else {
			solverSettings.printReport = false;
			solverSettings.options.minimizer_progress_to_stdout = false;
		}
		return solverSettings;
	}

	//----------
	StewartPlatform::Actuators::Actuators()
	{
		// Basic setup
		this->setName("Actuators");
		this->add(this->minimumLength);
		this->add(this->extension);

		// Initialise the actuators
		for (int i = 0; i < 6; i++) {
			auto actuator = std::make_shared<Actuator>();
			this->actuators[i] = actuator;

			// Parameters
			{
				actuator->value.setName(ofToString(i));
				actuator->value.set(0); // will default to minimum
				this->add(actuator->value);
			}

			// Joints
			{
				actuator->joints["upper"] = SA::System::Joint();
				actuator->joints["lower"] = SA::System::Joint();
			}

			// Draw function
			{
				auto actuatorWeak = std::weak_ptr<Actuator>(actuator);
				actuator->onDraw += [this, actuatorWeak]() {
					auto actuator = actuatorWeak.lock();
					auto view = glm::lookAt(actuator->joints["lower"].position
						, actuator->joints["upper"].position
						, glm::vec3(0, 1, 0));
					auto viewInverse = glm::inverse(view);

					auto bodyLength = actuator->value.getMin();

					ofPushMatrix();
					{
						ofMultMatrix(viewInverse);
						ofRotateDeg(90, -1, 0, 0);
						ofPushStyle();
						{
							// Actuator body
							ofPushMatrix();
							{
								ofTranslate(0, bodyLength / 2.0f, 0.0f);

								auto gb = ofMap(actuator->framesSinceChange, 0, 10, 0, 150, true);
								ofColor color(150, gb, gb);
								ofSetColor(color);
								material.setDiffuseColor(color);

								light.enable();
								material.begin();
								ofDrawCylinder(0.01, bodyLength);
								material.end();
								light.disable();
							}
							ofPopMatrix();

							// Draw limit 
							{
								auto maxExtension = actuator->value.getMax() - actuator->value.getMin();
								ofTranslate(0, actuator->value.get() - maxExtension);
								ofRotateDeg(90, 1, 0, 0);
								ofNoFill();
								ofDrawCircle(0, 0, 0.011);
							}

						}
						ofPopStyle();
					}
					ofPopMatrix();
				};
			}
		}

		// On actuator range change
		{
			auto changeCallback = [this](const float&) {
				const auto& min = this->minimumLength.get();
				const auto& extension = this->extension.get();
				const auto max = min + extension;

				for (auto actuator : this->actuators)
				{
					actuator->value.setMin(min);
					actuator->value.setMax(max);
					if (actuator->value.get() < min || actuator->value.get() > max) {
						actuator->value.set(ofClamp(actuator->value.get(), min, max));
					}
				}
			};
			this->minChangeListener = this->minimumLength.newListener(changeCallback);
			this->extensionChangeListener = this->extension.newListener(changeCallback);
			changeCallback(0.0f);
		}

		// On actuator value change
		{
			for (int i = 0; i < 6; i++) {
				auto changeCallback = [this, i](const float&) {
					this->actuators[i]->framesSinceChange = 0;
					this->onValueChange.notifyListeners();
				};
				this->actuators[i]->valueChangeListener = this->actuators[i]->value.newListener(changeCallback);
			}
		}
	}
	//----------
	StewartPlatform::StewartPlatform()
	{
#ifdef _DEBUG
		this->solveOptions.maxIterations.set(50);
#endif
		auto rebuildCallback = [this]() {
			this->markNeedsRebuild();
		};

		this->upperDeck = std::make_shared<Deck>();
		this->lowerDeck = std::make_shared<Deck>();

		auto initDeck = [this](std::shared_ptr<Deck> deck, const std::string& name)
		{
			deck->add(deck->diameter);
			deck->add(deck->jointSpacing);
			deck->add(deck->rotationOffset);
			deck->add(deck->rotationX);
			deck->setName(name);
			this->system.bodies[deck->getName()] = deck;
		};
		initDeck(upperDeck, "Upper deck");
		initDeck(lowerDeck, "Lower deck");


		this->actuators.onValueChange += [this]() {
			if (this->solveOptions.FKWhenActuatorChange) {
				this->needsFKSolve = true;
			}
		};
		this->upperDeck->onChange += rebuildCallback;
		this->lowerDeck->onChange += rebuildCallback;

		this->lowerDeck->diameter.set(1.14f);

		this->lowerDeck->rotateDeg(180, 0, 1, 0);
		this->upperDeck->setPosition({ 0, 1, 0 });
		this->upperDeck->diameter.set(1.6f);

		this->add(this->solveOptions);
		this->add(*this->upperDeck);
		this->add(*this->lowerDeck);
		this->add(this->weight);
		this->add(this->transform);
		this->add(this->actuators);
		this->add(this->test);
		this->add(this->counterWeight);

		// Init system other parts
		{
			// Add actuators
			for (auto& actuator : this->actuators.actuators)
			{
				this->system.bodies[actuator->value.getName()] = actuator;
			}

			// Connect joints on actuators
			{
				auto connectActuator = [this](int actuatorIndex, const std::string& lowerJoint, const std::string& upperJoint)
				{
					const auto actuatorName = this->actuators.actuators[actuatorIndex]->value.getName();
					this->system.jointConnections.push_back(
						{
							{
								actuatorName
								, "lower"
							}
							, {
								this->lowerDeck->getName()
								, lowerJoint
							}
						}
					);
					this->system.jointConnections.push_back(
						{
							{
								actuatorName
								, "upper"
							}
							, {
								this->upperDeck->getName()
								, upperJoint
							}
						}
					);
				};

				connectActuator(0, "A1", "B2");
				connectActuator(1, "A2", "C1");
				connectActuator(2, "B1", "C2");
				connectActuator(3, "B2", "A1");
				connectActuator(4, "C1", "A2");
				connectActuator(5, "C2", "B1");
			}

			// Add ground support (additional joint + set it as ground support)
			{
				this->lowerDeck->joints["Ground"].position = glm::vec3();
				this->system.groundSupports.push_back({
					{
						this->lowerDeck->getName()
						, "Ground"
					}
					});
			}
		}

		// Listen for transform change
		{
			auto transformChangeCallback = [this](float&) {
				this->markNewTransformParameters();
			};
			this->transform.translate.changeListenerX = this->transform.translate.x.newListener(transformChangeCallback);
			this->transform.translate.changeListenerY = this->transform.translate.y.newListener(transformChangeCallback);
			this->transform.translate.changeListenerZ = this->transform.translate.z.newListener(transformChangeCallback);
			this->transform.rotate.changeListenerX = this->transform.rotate.x.newListener(transformChangeCallback);
			this->transform.rotate.changeListenerY = this->transform.rotate.y.newListener(transformChangeCallback);
			this->transform.rotate.changeListenerZ = this->transform.rotate.z.newListener(transformChangeCallback);
		}

		// Listen for weight change
		{
			auto weightChangeCallback = [this](float&) {
				this->weight.isDirty = true;
			};
			this->weight.offsetYListener = this->weight.offsetY.newListener(weightChangeCallback);
			this->weight.offsetZListener = this->weight.offsetZ.newListener(weightChangeCallback);
			this->weight.massListener = this->weight.mass.newListener(weightChangeCallback);
		}

		// Listen for counter weight change
		{
			this->counterWeight.enabledChangeListener = this->counterWeight.enabled.newListener([this](bool&) {
				this->counterWeight.isDirty = true;
				});
			auto floatChange = [this](float&) {
				this->counterWeight.isDirty = true;
			};
			this->counterWeight.forceChangedListener = this->counterWeight.force.newListener(floatChange);

			auto vecChange = [this](glm::vec3&) {
				this->counterWeight.isDirty = true;
			};
			this->counterWeight.anchorPointChangeListener = this->counterWeight.anchorPoint.newListener(vecChange);
			this->counterWeight.upperDeckAttachmentListener = this->counterWeight.upperDeckAttachment.newListener(vecChange);
		}

		// Light
		{
			light.setDirectional();
			light.setOrientation(glm::vec3{ 180 - 45, 0, 0 });
			light.setAmbientColor(ofColor(100));
			light.setDiffuseColor(ofColor(150));
		}

		this->resetTransform();
	}

	//----------
	void
		StewartPlatform::update()
	{
		this->upperDeck->update();
		this->lowerDeck->update();

		if (this->transform.reset)
		{
			this->resetTransform();
			this->transform.reset = false;
		}

		bool needsRebuild = false;
		if (this->weight.isDirty)
		{
			this->rebuildWeight();
		}
		if (this->counterWeight.isDirty)
		{
			this->rebuildCounterweight();
		}

		if (this->needsTransformFromParameters)
		{
			this->transformFromParameters();
		}

		if (this->needsTransformToParameters)
		{
			this->transformToParameters();
		}

		if (this->needsRebuild)
		{
			this->rebuild(true);
		}

		if (this->needsFKSolve)
		{
			this->solveFK();
		}

		if (!this->forcesSolved && this->solveOptions.forcesWhenDirty)
		{
			this->needsForceSolve = true;
		}

		if (this->needsForceSolve)
		{
			this->solveForces();
		}

		for (auto& actuator : this->actuators.actuators)
		{
			actuator->framesSinceChange++;
		}

		if (this->test.cycleExtremes) {
			const auto& extremeIndex = (this->test.extremeIndex.get() + 1) % (1 << 6);
			this->test.extremeIndex.set(extremeIndex);
			for (int i = 0; i < 6; i++) {
				this->actuators.actuators[i]->value.set(
					(extremeIndex >> i) & 1
					? this->actuators.actuators[i]->value.getMin()
					: this->actuators.actuators[i]->value.getMax()
				);
			}
			this->fkSolved = false;
			int tries = 0;
			while (!this->fkSolved) {
				this->solveFK();
				tries++;
				if (tries > 100) {
					this->test.cycleExtremes = false;
					break;
				}
			}
		}

		this->checkActuatorLimits();
	}

	//----------
	void
		StewartPlatform::customDraw()
	{
		this->system.draw();
	}

	//----------
	void
		StewartPlatform::serialize(nlohmann::json& json)
	{
		{
			auto& jsonSolveOptions = json["solveOptions"];
			::serialize(jsonSolveOptions, this->solveOptions.printOutput);
			::serialize(jsonSolveOptions, this->solveOptions.maxIterations);
			::serialize(jsonSolveOptions, this->solveOptions.forcesWhenDirty);
			::serialize(jsonSolveOptions, this->solveOptions.IKWhenRebuild);
			::serialize(jsonSolveOptions, this->solveOptions.FKWhenActuatorChange);
		}

		{
			auto& jsonTransform = json["transform"];
			{
				auto& jsonTranslate = jsonTransform["translate"];
				::serialize(jsonTranslate, this->transform.translate.x);
				::serialize(jsonTranslate, this->transform.translate.y);
				::serialize(jsonTranslate, this->transform.translate.z);
			}
			{
				auto& jsonRotate = jsonTransform["rotate"];
				::serialize(jsonRotate, this->transform.rotate.x);
				::serialize(jsonRotate, this->transform.rotate.y);
				::serialize(jsonRotate, this->transform.rotate.z);
			}
		}

		{
			auto& jsonWeight = json["weight"];
			{
				::serialize(jsonWeight, this->weight.offsetY);
				::serialize(jsonWeight, this->weight.offsetZ);
				::serialize(jsonWeight, this->weight.mass);
			}
		}

		{
			auto& jsonCounterWeight = json["counterWeight"];
			{
				::serialize(jsonCounterWeight, this->counterWeight.enabled);
				::serialize(jsonCounterWeight, this->counterWeight.force);
				::serialize(jsonCounterWeight, this->counterWeight.anchorPoint);
				::serialize(jsonCounterWeight, this->counterWeight.upperDeckAttachment);
			}
		}

		this->upperDeck->serialize(json["upperDeck"]);
		this->lowerDeck->serialize(json["lowerDeck"]);
	}

	//----------
	void
		StewartPlatform::deserialize(const nlohmann::json& json)
	{
		{
			auto& jsonSolveOptions = json.at("solveOptions");
			::deserialize(jsonSolveOptions, this->solveOptions.printOutput);
			::deserialize(jsonSolveOptions, this->solveOptions.maxIterations);
			::deserialize(jsonSolveOptions, this->solveOptions.forcesWhenDirty);
			::deserialize(jsonSolveOptions, this->solveOptions.IKWhenRebuild);
			::deserialize(jsonSolveOptions, this->solveOptions.FKWhenActuatorChange);
		}

		{
			auto& jsonTransform = json.at("transform");
			{
				auto& jsonTranslate = jsonTransform["translate"];
				::deserialize(jsonTranslate, this->transform.translate.x);
				::deserialize(jsonTranslate, this->transform.translate.y);
				::deserialize(jsonTranslate, this->transform.translate.z);
			}
			{
				auto& jsonRotate = jsonTransform["rotate"];
				::deserialize(jsonRotate, this->transform.rotate.x);
				::deserialize(jsonRotate, this->transform.rotate.y);
				::deserialize(jsonRotate, this->transform.rotate.z);
			}
		}

		{
			auto& jsonWeight = json.at("weight");
			{
				::deserialize(jsonWeight, this->weight.offsetY);
				::deserialize(jsonWeight, this->weight.offsetZ);
				::deserialize(jsonWeight, this->weight.mass);
			}
		}

		if (json.contains("counterWeight")) {
			auto& jsonCounterWeight = json["counterWeight"];
			{
				::deserialize(jsonCounterWeight, this->counterWeight.enabled);
				::deserialize(jsonCounterWeight, this->counterWeight.force);
				::deserialize(jsonCounterWeight, this->counterWeight.anchorPoint);
				::deserialize(jsonCounterWeight, this->counterWeight.upperDeckAttachment);
			}
		}

		this->upperDeck->deserialize(json.at("upperDeck"));
		this->lowerDeck->deserialize(json.at("lowerDeck"));
	}

	//----------
	void
		StewartPlatform::resetTransform()
	{
		this->transform.translate.x.set(0);
		this->transform.translate.y.set(0);
		this->transform.translate.z.set(0.6);
		this->transform.rotate.x.set(0);
		this->transform.rotate.y.set(0);
		this->transform.rotate.z.set(0);
	}

	//----------
	void
		StewartPlatform::markNeedsRebuild()
	{
		this->needsRebuild = true;
	}

	//----------
	void
		StewartPlatform::markNeedsFKSolve()
	{
		this->needsFKSolve = true;
		this->fkSolved = false;
	}

	//----------
	void
		StewartPlatform::markNeedsForceSolve()
	{
		this->needsForceSolve = true;
		this->forcesSolved = false;
	}

	//----------
	void
		StewartPlatform::markNewTransformMatrix()
	{
		this->needsTransformToParameters = true;
		this->needsRebuild = true;
	}

	//----------
	void
		StewartPlatform::markNewTransformParameters()
	{
		this->needsTransformFromParameters = true;
	}


	//----------
	void
		StewartPlatform::rebuild(bool allowIKSolve)
	{
		// Update the joint positions on actuators + set the actuator position to its lower position
		for (auto actuator : this->actuators.actuators)
		{
			//Find the connection related to the lower joint
			auto connectedJointAddress = this->system.findConnectedJoint({
				actuator->value.getName()
				, "lower"
				});
			auto lowerPosition = this->system.bodies[connectedJointAddress.bodyName]->getJointPosition(connectedJointAddress.jointName);

			connectedJointAddress = this->system.findConnectedJoint({
				actuator->value.getName()
				, "upper"
				});
			auto upperPosition = this->system.bodies[connectedJointAddress.bodyName]->getJointPosition(connectedJointAddress.jointName);

			actuator->setPosition(lowerPosition);
			actuator->joints["upper"].position = upperPosition - lowerPosition;
		}

		// Clear all forces (they will be invalid now)
		for (auto& joint : this->upperDeck->joints) {
			joint.second.force = glm::vec3(0, 0, 0);
		}

		if (allowIKSolve) {
			this->fkSolved = false;
		}
		this->forcesSolved = false;

		if (this->solveOptions.IKWhenRebuild && allowIKSolve) {
			this->solveIK();
		}

		this->needsRebuild = false;
	}

	//----------
	void
		StewartPlatform::rebuildWeight()
	{
		this->upperDeck->loads["Weight"].position.y = this->weight.offsetY;
		this->upperDeck->loads["Weight"].position.z = this->weight.offsetZ;
		this->upperDeck->loads["Weight"].force.y = this->weight.mass * -9.81;
		this->weight.isDirty = false;
		this->markNeedsRebuild();
	}

	//----------
	void
		StewartPlatform::rebuildCounterweight()
	{
		if (this->counterWeight.enabled.get()) {
			this->upperDeck->loads["Counterweight"].position = this->counterWeight.upperDeckAttachment.get();
			auto forceDirection = glm::normalize(this->counterWeight.anchorPoint.get() - this->upperDeck->getLoadPosition("Counterweight"));
			this->upperDeck->loads["Counterweight"].force = forceDirection * this->counterWeight.force.get();
		}
		else {
			auto findCounterWeight = this->upperDeck->loads.find("Counterweight");
			if (findCounterWeight != this->upperDeck->loads.end()) {
				this->upperDeck->loads.erase(findCounterWeight);
			}
		}
		this->counterWeight.isDirty = false;

		this->markNeedsRebuild();
	}

	//----------
	void
		StewartPlatform::checkActuatorLimits()
	{
		// There are 'rounding errors' by going full loop through optimiser
		float offsetFactor = 1e-3;

		bool needsFK = false;
		for (auto actuator : this->actuators.actuators) {
			if (actuator->value.get() > actuator->value.getMax()) {
				actuator->value.set(actuator->value.getMax() - offsetFactor);
				needsFK = true;
			}
			if (actuator->value.get() < actuator->value.getMin()) {
				actuator->value.set(actuator->value.getMin() + offsetFactor);
				needsFK = true;
			}
		}

		if (needsFK) {
			this->solveFK();
		}
	}
}