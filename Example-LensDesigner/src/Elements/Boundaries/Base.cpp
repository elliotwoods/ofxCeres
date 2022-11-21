#include "pch_ofApp.h"
#include "Base.h"

namespace Elements {
	namespace Boundaries {
		//----------
		Base::Base()
		{
			this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
				args.inspector->addParameterGroup(this->parameters);
			};
		}

		//----------
		void
			Base::setEntranceIOR(float value)
		{
			this->parameters.entranceIOR.set(value);
		}

		//----------
		void
			Base::setExitIOR(float value)
		{
			this->parameters.exitIOR.set(value);
		}
	}
}