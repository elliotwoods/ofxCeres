#pragma once

#include "OpticalElement.h"
#include "Ray.h"

namespace Models {
	template<typename T>
	struct FlatBoundary_ : OpticalElement_<T> {
		glm::tvec2<T> center;
		glm::tvec2<T> normalTowardsExit;
		T exitVsEntranceIOR;

		// ReMarkable notes November 2022 Page 7
		T intersectU(const Ray_<T>& ray)
		{
			auto u = (glm::dot(this->center, this->normalTowardsExit) - glm::dot(ray.s, this->normalTowardsExit))
				/ glm::dot(ray.t, this->normalTowardsExit);
			return u;
		}
		bool intersect(const Ray_<T>& ray, glm::tvec2<T>& point) override
		{
			auto u = this->intersectU(ray);
			point = ray.s + u * ray.t;
			return u >= (T) 0.0;
		}

		shared_ptr<Ray_<T>>
			interact(const Ray_<T> & incoming) override
		{
			auto intersectU = this->intersectU(incoming);
			if (intersectU <= (T) 0) {
				return nullptr;
			}
			auto newRay = make_shared<Ray_<T>>();
			{
				newRay->s = incoming.s + intersectU * incoming.t;
				newRay->t = ofxCeres::VectorMath::refract(incoming.t
					, this->normalTowardsExit
					, this->exitVsEntranceIOR);
			}
			return newRay;
		}

		template<typename T2>
		shared_ptr<FlatBoundary_<T2>> castTo()
		{
			auto instance2 = make_shared<FlatBoundary_<T2>>();
			{
				instance2->center = (glm::tvec2<T2>) this->center;
				instance2->normalTowardsExit = (glm::tvec2<T2>) this->normalTowardsExit;
				instance2->exitVsEntranceIOR = (T2) this->exitVsEntranceIOR;
			}
			return instance2;
		}
	};

	typedef FlatBoundary_<float> FlatBoundary;
}