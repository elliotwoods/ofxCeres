#pragma once

#include "ofxCvGui.h"
#include "Elements/Base.h"
#include "Models/Ray.h"

struct RayChainLink {
	Models::Ray ray;
	shared_ptr<Elements::Base> source;
	shared_ptr<RayChainLink> nextRay;
};

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

	void test();
	void drawTest();
protected:
	struct {
		vector<shared_ptr<RayChainLink>> rayChains;
	} testResults;
};