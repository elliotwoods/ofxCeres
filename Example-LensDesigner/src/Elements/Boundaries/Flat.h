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
			shared_ptr<Models::FlatBoundary_<T>>
			getModel() const
			{
				auto model = make_shared<Models::FlatBoundary_<T>>();
				model->center = this->getPosition();
				{
					model->normalTowardsExit.x = -sin(this->parameters.angle.get() * DEG_TO_RAD);
					model->normalTowardsExit.y = cos(this->parameters.angle.get() * DEG_TO_RAD);
				}
				model->exitVsEntranceIOR = Boundaries::Base::parameters.exitIOR.get()
					/ Boundaries::Base::parameters.entranceIOR.get();
				return model;
			}

			template<typename T>
			void
			setModel(shared_ptr<Models::FlatBoundary_<T>>) const
			{
			}

			shared_ptr<Models::OpticalElement_<float>> getOpticalModelUntyped() const override;
			void setOpticalModelUntyped(shared_ptr<Models::OpticalElement_<float>>) override;

			struct : ofParameterGroup {
				ofParameter<float> width{ "Width", 0.06, 0, 5 };
				ofParameter<float> angle{ "Angle", 0, 0, 360 };
				PARAM_DECLARE("Flat", width, angle)
			} parameters;
		};
	}
}