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

	// setup the multiple choice
	auto selectConstraint = make_shared<ofxCvGui::Widgets::MultipleChoice>(this->constraint.getName(), this->constraint.get().getOptionStrings());
	{
		selectConstraint->onValueChange += [this](const int& index) {
			auto constraint = this->constraint.get();
			constraint.fromIndex(index);
			this->constraint.set(constraint);
		};
		vector<string> glyphs{ u8"\uF31E", u8"\uf05b", u8"\uf13d", u8"\uf424"};
		selectConstraint->setGlyphs(glyphs);
	}

	auto children = vector<ofxCvGui::ElementPtr>({
		make_shared<ofxCvGui::Widgets::EditableValue<string>>(this->name)
		, make_shared<ofxCvGui::Widgets::EditableValue<glm::vec3>>(this->position)
		, selectConstraint
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

	{
		auto height = 0.0f;
		for (auto element : element->getChildren()) {
			height += element->getHeight();
		}
		height += 5.0f;
		element->setHeight(height);
	}

	return element;
}