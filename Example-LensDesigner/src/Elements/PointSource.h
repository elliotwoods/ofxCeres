#pragma once

#include "Elements/Base.h"
#include "Models/Ray.h"

namespace Elements {
	class PointSource : public Base {
	public:
		PointSource();
		string getTypeName() const override;
		string getGlyph() const override;

		template<typename T>
		vector<Models::Ray_<T>>
			emitRays(size_t count)
		{
			vector<Models::Ray_<T>> rays;
			const auto & spread = this->parameters.spread.get() * DEG_TO_RAD;
			auto theta = this->parameters.angle.get() * DEG_TO_RAD - spread / 2.0f;
			auto step = spread / (float)count;

			Models::Ray_<T> ray;
			ray.s = this->getPosition();
			for (size_t i = 0; i < count; i++) {
				ray.t.x = cos(theta);
				ray.t.y = sin(theta);
				rays.push_back(ray);
				theta += step;
			}
			return rays;
		}

		struct : ofParameterGroup {
			ofParameter<float> angle{ "Angle (deg)", 90, 0, 360 };
			ofParameter<float> spread{ "Spread (deg)", 60, 0, 360 };
			PARAM_DECLARE("PointSource", angle, spread);
		} parameters;
	};
}