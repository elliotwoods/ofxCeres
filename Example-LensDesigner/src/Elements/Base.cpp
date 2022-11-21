#include "pch_ofApp.h"
#include "Base.h"

namespace Elements {
	//----------
	void
		Base::init()
	{
		this->parameters.name = this->getTypeName();
		this->manageParameters(this->parameters);
	}

	//----------
	string
		Base::getName() const
	{
		return this->parameters.name.get();
	}

	//----------
	glm::vec2
		Base::getPosition() const
	{
		return {
			this->parameters.position.x.get()
			, this->parameters.position.y.get()
		};
	}

	//----------
	void
		Base::setPosition(const glm::vec2& position)
	{
		this->parameters.position.x = position.x;
		this->parameters.position.y = position.y;
	}

	//----------
	void
		Base::drawWorldSpace() const
	{

		ofxCvGui::Utils::drawTextAnnotation(this->getName()
			, { this->getPosition(), 0.0f });

		ofPushMatrix();
		{
			ofTranslate(this->getPosition());
			this->drawObjectSpace();
		}
		ofPopMatrix();
	}

	//----------
	void
		Base::drawObjectSpace() const
	{
		this->onDrawObjectSpace.notifyListeners();
	}

	//----------
	void
		Base::manageParameters(ofParameterGroup& parameters)
	{
		this->onPopulateInspector += [this, &parameters](ofxCvGui::InspectArguments& args) {
			args.inspector->addParameterGroup(parameters);
		};
	}
}
