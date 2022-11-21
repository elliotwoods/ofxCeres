#pragma once

#include "Ray.h"

namespace Models {
	template<typename T>
	struct FlatBoundary_ {
		glm::tvec2<T> center;
		glm::tvec2<T> normalTowardsExit;

		T exitVsEntranceIOR;

		// ReMarkable notes November 2022 Page 7
		T intersectU(const Ray_<T>& ray) {
			auto u = (glm::dot(this->center, this->normalTowardsExit) - glm::dot(ray.s, this->normalTowardsExit))
				/ glm::dot(ray.t, this->normalTowardsExit);
			return u;
		}
		glm::tvec2<T> intersect(const Ray_<T>& ray) {
			auto u = this->intersectU(ray);
			return ray.s + u * ray.t;
		}

		shared_ptr<Ray_<T>>
			interact(const Ray_<T> & incoming) {
			auto intersectU = this->intersectU(incoming);
			if (intersectU <= 0) {
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
		FlatBoundary_<T2> castTo()
		{
			FlatBoundary_<T2> instance2;
			{
				instance2.center = (glm::tvec2<T2>) this->center;
				instance2.normalTowardsExit = (glm::tvec2<T2>) this->normalTowardsExit;
				instance2.entranceIOR = (T2) this->entranceIOR;
				instance2.exitIOR = (T2) this->exitIOR;
			}
			return instance2;
		}
	};

	typedef FlatBoundary_<float> FlatBoundary;
}