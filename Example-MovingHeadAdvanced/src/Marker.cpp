#include "pch_ofApp.h"
#include "Marker.h"

//----------
Marker::Marker()
{

}

//----------
string
Marker::getTypeName() const
{
	return "Marker";
}

//----------
ofxCvGui::ElementPtr
Marker::getDataDisplay()
{
	auto element = ofxCvGui::makeElement();

	auto children = vector<ofxCvGui::ElementPtr>({
		make_shared<ofxCvGui::Widgets::EditableValue<string>>(this->name)
		, make_shared<ofxCvGui::Widgets::EditableValue<glm::vec3>>(this->position)
		});

	for (auto child : children) {
		element->addChild(child);
	}

	element->onBoundsChange += [children](ofxCvGui::BoundsChangeArguments& args) {
		auto bounds = args.localBounds;
		for (auto element : children) {
			bounds.height = element->getHeight();
			element->setBounds(bounds);
			bounds.y += bounds.height;
		}
	};

	element->setHeight(element->getChildren().size() * 40.0f + 5.0f);

	return element;
}