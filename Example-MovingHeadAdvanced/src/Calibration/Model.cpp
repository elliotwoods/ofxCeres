#include "pch_ofApp.h"
#include "Model.h"
#include "ofxCeres.h"
#include "DMX/MovingHead.h"
#include "Scene.h"

namespace Calibration {
	//----------
	Model::Parameters::Parameters()
	{
		this->setName("Model");
		this->add(this->translation);
		this->add(this->rotationVector);
		this->add(this->panDistortion);
		this->add(this->tiltDistortion);
		this->add(this->residual);
	}

	//----------
	Model::Model(DMX::MovingHead& movingHead)
		: movingHead(movingHead)
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;

	}

	//----------
	string
	Model::getTypeName() const
	{
		return "Calibration::Model";
	}

	//---------
	void
	Model::serialize(nlohmann::json& json)
	{
		Data::serialize(json, this->parameters);

		{
			auto& focusModel = json["focusModel"];
			for (const auto& coefficient : this->focusModel.coefficients) {
				focusModel.push_back(coefficient);
			}
		}
	}

	//---------
	void
	Model::deserialize(const nlohmann::json& json)
	{
		Data::deserialize(json, this->parameters);

		if (json.contains("focusModel")) {
			const auto& jsonFocusModel = json["focusModel"];
			if (jsonFocusModel.size() == this->focusModel.coefficients.size()) {
				for (size_t i = 0; i < this->focusModel.coefficients.size(); i++) {
					this->focusModel.coefficients[i] = jsonFocusModel[i].get<float>();
				}
			}
		}
	}

	//---------
	void
	Model::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;

		inspector->addTitle("Model");

		// We manually populate the dialog because the last value isn't editable
		inspector->addEditableValue<glm::vec3>(this->parameters.translation);
		inspector->addEditableValue<glm::vec3>(this->parameters.rotationVector);
		inspector->addEditableValue<glm::vec3>(this->parameters.panDistortion);
		inspector->addEditableValue<glm::vec3>(this->parameters.tiltDistortion);
		inspector->addLiveValue<float>(this->parameters.residual);

		inspector->addButton("Reset", [this]() {
			this->parameters.translation.set(glm::vec3(0.0));
			this->parameters.rotationVector.set(glm::vec3(0.0));
			this->parameters.panDistortion.set({ 0, 1, 0 });
			this->parameters.tiltDistortion.set({ 0, 1, 0 });
			this->parameters.residual.set(0.0);
			});
		inspector->addButton("Take cursor position", [this]() {
			auto position = Scene::X()->getPanel()->getCamera().getCursorWorld();
			this->parameters.translation.set(position);
			}, ' ');
	}


	//---------
	glm::mat4
	Model::getTransform() const
	{
		return ofxCeres::VectorMath::createTransform(this->parameters.translation.get()
			, this->parameters.rotationVector.get());
	}

	//---------
	glm::vec3
	Model::getPosition() const
	{
		return this->parameters.translation.get();
	}

	//---------
	void
	Model::applyRotation(const glm::quat& rotation)
	{
		{
			auto translation = this->parameters.translation.get();
			translation = rotation * translation;
			this->parameters.translation.set(translation);
		}

		{
			auto rotationVector = this->parameters.rotationVector.get();
			auto rotationPre = ofxCeres::VectorMath::eulerToQuat(rotationVector);
			auto rotationPost = rotation * rotationPre;
			rotationVector = glm::eulerAngles(rotationPost);
			this->parameters.rotationVector.set(rotationVector);
		}
	}

	//---------
	glm::vec2
	Model::getPanTiltForWorldTarget(const glm::vec3& world
			, const glm::vec2& currentPanTilt) const
	{
		auto objectSpacePosition4 = glm::inverse(this->getTransform()) * glm::vec4(world, 1.0f);
		auto objectSpacePosition = (glm::vec3)(objectSpacePosition4 / objectSpacePosition4.w);

		auto panTiltObject = ofxCeres::VectorMath::getPanTiltToTargetInObjectSpace(objectSpacePosition);

		// build up the options
		vector<glm::vec2> panTiltOptions;
		{
			// basic option
			panTiltOptions.push_back(panTiltObject);

			// alternative pan options
			{
				for (float pan = panTiltObject.x - 360.0f; pan >= this->movingHead.parameters.pan.getMin(); pan -= 360.0f) {
					panTiltOptions.push_back(glm::vec2(pan, panTiltObject.y));
				}
				for (float pan = panTiltObject.x + 360.0f; pan <= this->movingHead.parameters.pan.getMax(); pan += 360.0f) {
					panTiltOptions.push_back(glm::vec2(pan, panTiltObject.y));
				}
			}

			// flipped tilt options
			{
				for (float pan = panTiltObject.x - 180.0f; pan >= this->movingHead.parameters.pan.getMin(); pan -= 360.0f) {
					panTiltOptions.push_back(glm::vec2(pan, -panTiltObject.y));
				}
				for (float pan = panTiltObject.x + 180.0f; pan <= this->movingHead.parameters.pan.getMax(); pan += 360.0f) {
					panTiltOptions.push_back(glm::vec2(pan, -panTiltObject.y));
				}
			}
		}

		// search through options for closest one
		auto minDistance2 = std::numeric_limits<float>::max();
		glm::vec2 bestOption = panTiltObject;
		for (const auto& panTiltOption : panTiltOptions) {
			auto distance2 = glm::distance2(panTiltOption, currentPanTilt);
			if (distance2 < minDistance2) {
				bestOption = panTiltOption;
				minDistance2 = distance2;
			}
		}

		// apply distortion
		auto signalPanTilt = this->panTiltIdealToSignal(bestOption);

		// presumes we are inside range (TODO : in edge cases, these values might be outside range because of distortion)
		return signalPanTilt;
	}

	//---------
	glm::vec2
	Model::panTiltIdealToSignal(const glm::vec2& ideal) const
	{
		auto panOptions = ofxCeres::VectorMath::powerSeries2Inverse(ideal.x, (float*)&this->parameters.panDistortion.get());
		auto tiltOptions = ofxCeres::VectorMath::powerSeries2Inverse(ideal.y, (float*)&this->parameters.tiltDistortion.get());

		auto signal = glm::vec2{
			ofxCeres::VectorMath::pickClosest(ideal.x, panOptions.first, panOptions.second)
			, ofxCeres::VectorMath::pickClosest(ideal.y, tiltOptions.first, tiltOptions.second)
		};

		auto recalcIdeal = this->panTiltSignalToIdeal(signal);
		return signal;
	}

	//---------
	glm::vec2
	Model::panTiltSignalToIdeal(const glm::vec2& signal) const
	{
		return {
			ofxCeres::VectorMath::powerSeries2(signal.x, (float*)&this->parameters.panDistortion.get())
			, ofxCeres::VectorMath::powerSeries2(signal.y, (float*)&this->parameters.tiltDistortion.get())
		};
	}

	//---------
	ofxCeres::Models::DistortedMovingHead::Solution
	Model::getDistortedMovingHeadSolution() const
	{
		ofxCeres::Models::DistortedMovingHead::Solution solution;

		solution.basicSolution = ofxCeres::Models::MovingHead::Solution{
		this->parameters.translation.get()
			, this->parameters.rotationVector.get()
		};

		const auto& panDistortion = this->parameters.panDistortion.get();
		const auto& tiltDistortion = this->parameters.tiltDistortion.get();
		for (int i = 0; i < OFXCERES_DISTORTEDMOVINGHEAD_PARAMETER_COUNT; i++) {
			solution.panDistortion[i] = panDistortion[i];
			solution.tiltDistortion[i] = tiltDistortion[i];
		}

		return solution;
	}

	//---------
	void
	Model::setDistortedMovingHeadSolution(const ofxCeres::Models::DistortedMovingHead::Solution& solution)
	{
		this->parameters.translation = solution.basicSolution.translation;
		this->parameters.rotationVector = solution.basicSolution.rotationVector;
		this->parameters.panDistortion.set({
			solution.panDistortion[0]
			, solution.panDistortion[1]
			, solution.panDistortion[2]
			});
		this->parameters.tiltDistortion.set({
			solution.tiltDistortion[0]
			, solution.tiltDistortion[1]
			, solution.tiltDistortion[2]
			});
	}
}