#include "pch_ofApp.h"
#include "StewartPlatform.h"
#include "Solvers/StewartPlatformForces.h"
#include "Solvers/StewartPlatformFK.h"
#include <glm/gtx/matrix_decompose.hpp>

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
		StewartPlatform::solveForces()
	{
		{
			auto solverSettings = Solvers::StewartPlatformForces::defaultSolverSettings();
			{
				solverSettings.options.function_tolerance = 0.0f;
				solverSettings.options.max_num_iterations = 1000;
#ifdef _DEBUG
				//solverSettings.printReport = true;
				//solverSettings.options.minimizer_progress_to_stdout = true;
#else
				solverSettings.printReport = false;
				solverSettings.options.minimizer_progress_to_stdout = false;
#endif
			}
			auto result = Solvers::StewartPlatformForces::solve(*this, true, solverSettings);
			this->forcesSolved = result.success;
			if (result.success) {
				this->needsForceSolve = false;
			}
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

		{
			auto solverSettings = Solvers::StewartPlatformForces::defaultSolverSettings();
			{
				solverSettings.options.function_tolerance = 0.0f;
				solverSettings.options.max_num_iterations = 1000;
#ifdef _DEBUG
				//solverSettings.printReport = true;
				//solverSettings.options.minimizer_progress_to_stdout = true;
#else
				solverSettings.printReport = false;
				solverSettings.options.minimizer_progress_to_stdout = false;
#endif
			}
			auto result = Solvers::StewartPlatformFK::solve(*this, true, solverSettings);
			this->fkSolved = result.success;
			if (result.success) {
				this->needsFKSolve = false;
			}
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
		auto B = TWO_PI / 3.0f;;
		auto C = 2.0f * TWO_PI / 3.0f;

		this->joints["A1"].position = vertex(A - this->jointAngleOffset);
		this->joints["A2"].position = vertex(A + this->jointAngleOffset);
		this->joints["B1"].position = vertex(B - this->jointAngleOffset);
		this->joints["B2"].position = vertex(B + this->jointAngleOffset);
		this->joints["C1"].position = vertex(C - this->jointAngleOffset);
		this->joints["C2"].position = vertex(C + this->jointAngleOffset);

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
	StewartPlatform::Actuators::Actuators()
	{
		// Basic setup
		this->setName("Actuators");
		this->add(this->minimumLength);
		this->add(this->maximumLength);

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
		}

		// On actuator range change
		{
			auto changeCallback = [this](const float&) {
				auto min = this->minimumLength.get();
				auto max = this->maximumLength.get();

				if (min > max) {
					this->maximumLength.set(min);
					max = min;
				}

				for (auto actuator : this->actuators)
				{
					actuator->value.setMin(min);
					actuator->value.setMax(max);
					if (actuator->value.get() < min || actuator->value.get() > max) {
						actuator->value.set(ofClamp(actuator->value.get(), min, max));
					}
				}
			};
			this->maxChangeListener = this->minimumLength.newListener(changeCallback);
			this->minChangeListener = this->maximumLength.newListener(changeCallback);
			changeCallback(0.0f);
		}

		// On actuator value change
		{
			auto changeCallback = [this](const float&) {
				this->onValueChange.notifyListeners();
			};
			for (int i = 0; i < 6; i++) {
				valueChangeListener[i] = this->actuators[i]->value.newListener(changeCallback);
			}
		}
	}
	//----------
	StewartPlatform::StewartPlatform()
	{
		auto rebuildCallback = [this]() {
			this->markNeedsRebuild();
		};

		this->upperDeck = std::make_shared<Deck>();
		this->lowerDeck = std::make_shared<Deck>();

		auto initDeck = [this](std::shared_ptr<Deck> deck, const std::string& name)
		{
			deck->add(deck->diameter);
			deck->add(deck->jointSpacing);
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

		this->lowerDeck->rotateDeg(180, 0, 1, 0);
		this->upperDeck->setPosition({ 0, 1, 0 });
		this->upperDeck->diameter.set(0.7f);

		this->add(this->solveOptions);
		this->add(*this->upperDeck);
		this->add(*this->lowerDeck);
		this->add(this->weight);
		this->add(this->transform);
		this->add(this->actuators);

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
				this->markNeedsRebuild();
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
			this->weight.offsetListener = this->weight.offset.newListener(weightChangeCallback);
			this->weight.massListener = this->weight.mass.newListener(weightChangeCallback);
		}
	}

	//----------
	void
		StewartPlatform::update()
	{
		this->upperDeck->update();
		this->lowerDeck->update();

		bool needsRebuild = false;
		if (this->weight.isDirty)
		{
			this->rebuildWeight();
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
	}

	//----------
	void
		StewartPlatform::customDraw()
	{
		this->system.draw();
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
		StewartPlatform::rebuild(bool allowIKSolve)
	{
		// Update the upper deck transform
		{
			const auto translation = glm::vec3(this->transform.translate.x.get()
				, this->transform.translate.y.get()
				, this->transform.translate.z.get());

			const auto rotationVector = glm::vec3(this->transform.rotate.x.get() * DEG_TO_RAD
				, this->transform.rotate.y.get() * DEG_TO_RAD
				, this->transform.rotate.z.get() * DEG_TO_RAD);

			const auto orientation = ofxCeres::VectorMath::eulerToQuat(rotationVector);

			this->upperDeck->setOrientation(orientation);
			this->upperDeck->setPosition(translation);
		}

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
		for (auto & joint : this->upperDeck->joints) {
			joint.second.force = glm::vec3(0, 0, 0);
		}

		this->fkSolved = false;
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
		this->upperDeck->loads["Weight"].position.z = this->weight.offset;
		this->upperDeck->loads["Weight"].force.y = this->weight.mass * -9.81;
		this->weight.isDirty = false;
		this->markNeedsRebuild();
	}
}