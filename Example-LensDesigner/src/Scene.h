#pragma once

#include "ofxCvGui.h"
#include "Elements/Base.h"
#include "Models/Ray.h"
#include "Models/OpticalSystem.h"

class Scene : public ofxCvGui::IInspectable {
public:
	Scene();
	void update();
	void inspect(ofxCvGui::InspectArguments&);
	shared_ptr<ofxCvGui::Panels::WorldManaged> panel;

	vector<shared_ptr<Elements::Base>> opticalElements;
	
	shared_ptr<Elements::Base>
		getOpticalElementByName(const std::string& name)
	{
		for (auto element : this->opticalElements) {
			if (element->getName() == name) {
				return element;
			}
		}
		return nullptr;
	}

	template<typename T>
	shared_ptr<T>
		getOpticalElementByType()
	{
		for (auto element : this->opticalElements) {
			auto typedElement = dynamic_pointer_cast<T>(element);
			if (typedElement) {
				return typedElement;
			}
		}
		return nullptr;
	}

	void cast();
	void drawPreviewRays();

	Models::OpticalSystem getOpticalSystem() const;
	void solve();
protected:
	struct {
		vector<Models::RayChain> rayChains;
	} preview;

	struct : ofParameterGroup {
		ofParameter<int> resolution{ "Resolution", 64 };
		ofParameter<bool> continuousSolve{ "Continuous solve", false };

		struct : ofParameterGroup {
			ofParameter<float> brightness{ "Brightness", 50.0f/255.0f, 0.0f, 1.0f };
			ofParameter<float> finalLength{ "Final length", 2.0f, 0.1f, 10.0f };
			PARAM_DECLARE("Preview", brightness, finalLength);
		} preview;
		PARAM_DECLARE("Scene", resolution, continuousSolve, preview);
	} parameters;

	ofxCeres::ParameterisedSolverSettings solverSettings;
};