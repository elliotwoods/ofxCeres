#pragma once

#include "Ray.h"

namespace Models {
	template<typename T>
	struct OpticalElement_ {
	public:
		virtual bool intersect(const Ray_<T>& ray, glm::tvec2<T>& point) = 0;
		virtual shared_ptr<Ray_<T>> interact(const Ray_<T>& ray) = 0;
		
		virtual size_t getParameterCount() const {
			return 0;
		}

		virtual void setParameters(const T*) {
		}

		virtual void getParameters(T*) {
		}
		
		template<typename T2>
		void copyBaseTo(shared_ptr<OpticalElement_<T2>> other)
		{
			other->configureParameters = this->configureParameters;
		}

		// Optionally defined function for configuring parameters
		std::function<void(ceres::Problem&, double*)> configureParameters;
	};
}
