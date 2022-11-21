#pragma once

#include "Base.h"

#include "Models/FlatBoundary.h"

namespace Elements {
	namespace Boundaries {
		class Flat : public Base {
		public:
			Flat();
			string getTypeName() const override;
			string getGlyph() const override;

			template<typename T>
			Models::FlatBoundary_<T>
			getModel() {
				Models::FlatBoundary_<T> model;
				model.center = this->getPosition();
				{
					model.normalTowardsExit.x = -sin(this->parameters.angle.get() * DEG_TO_RAD);
					model.normalTowardsExit.y = cos(this->parameters.angle.get() * DEG_TO_RAD);
				}
				model.exitVsEntranceIOR = Boundaries::Base::parameters.exitIOR.get()
					/ Boundaries::Base::parameters.entranceIOR.get();
				return model;
			}
		protected:
			struct : ofParameterGroup {
				ofParameter<float> width{ "Width", 1.0, 0, 5 };
				ofParameter<float> angle{ "Angle", 0, 0, 360 };
				PARAM_DECLARE("Flat", width, angle)
			} parameters;
		};
	}
}