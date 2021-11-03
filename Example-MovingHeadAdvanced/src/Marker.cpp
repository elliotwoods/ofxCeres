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
string
Marker::getGlyphForConstraint(const Constraint& constraint)
{
	switch (constraint.get()) {
	case Constraint::Free:
		return "";
	case Constraint::Origin:
		return u8"\uf05b";
	case Constraint::Fixed:
		return u8"\uf13d";
	case Constraint::Plane:
		return u8"\uf853";
	}
}

//----------
ofxCvGui::ElementPtr
Marker::getDataDisplay()
{
	auto element = ofxCvGui::makeElement();

	// setup the multiple choice
	auto selectConstraint = make_shared<ofxCvGui::Widgets::MultipleChoice>(this->constraint.getName(), this->constraint.get().getOptionStrings());
	{
		selectConstraint->entangleManagedEnum(this->constraint);

		vector<string> glyphs;
		auto optionsCount = this->constraint.get().getOptionStrings().size();
		for (size_t i = 0; i < optionsCount; i++) {
			glyphs.push_back(Marker::getGlyphForConstraint((Constraint::Options) i));
		}
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