#pragma once

#include "Elements/Base.h"

namespace Elements {
	namespace Boundaries {
		class Base : public Elements::Base {
		public:
			Base();
			void setEntranceIOR(float);
			void setExitIOR(float);
		protected:
			struct : ofParameterGroup {
				ofParameter<float> entranceIOR{ "Entrance IOR", 1.0, 0.0, 5.0 };
				ofParameter<float> exitIOR{ "Exit IOR", 1.5, 0.0, 5.0 };
				PARAM_DECLARE("Boundary", entranceIOR, exitIOR);
			} parameters;
		};
	}
}