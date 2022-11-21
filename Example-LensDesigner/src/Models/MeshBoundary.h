#pragma once

#include "Ray.h"
#include <map>

namespace Models {
	template<typename T>
	struct MeshBoundary_ {
		std::map<float, T> vertices;

		T exitVsEntranceIOR;

		static glm::tvec2<T>
		intersectUV(const Ray_<T>& ray
			, const glm::tvec2<T>& p1
			, const glm::tvec2<T>& p2
			, const glm::tvec2<T>& normalTowardsExit)
		{
			glm::tvec2<T> uv;
			uv.x = (glm::dot(p1, normalTowardsExit) - glm::dot(ray.s, normalTowardsExit))
				/ glm::dot(ray.t, normalTowardsExit);
			const auto point = ray.s + ray.t * uv.x;
			uv.y = glm::dot(point - p1, p2 - p1) / glm::dot(p2 - p1, p2 - p1);

			return uv;
		}

		// ReMarkable notes November 2022 Page 8
		struct IntersectUVResult {
			glm::tvec2<T> uv;
			glm::tvec2<T> p1;
			glm::tvec2<T> p2;
			glm::tvec2<T> normalTowardsExit;
		};

		bool intersectUV(const Ray_<T>& ray, IntersectUVResult& intersectUVResult)
		{
			// Iterate through line segments
			for (auto v1 = this->vertices.begin(); v1 != this->vertices.end(); v1++) {
				auto v2 = v1;
				{
					v2++;
					if (v2 == this->vertices.end()) {
						break;
					}
				}

				const glm::tvec2<T> p1((T)v1->first, v1->second);
				const glm::tvec2<T> p2((T)v2->first, v2->second);

				auto vectorInSurface = glm::normalize(p2 - p1);
				auto normalTowardsExit = glm::normalize(ray.t - glm::dot(ray.t, vectorInSurface) * vectorInSurface);

				// Test if in positive ray direction and within line segment
				auto uv = MeshBoundary_<T>::intersectUV(ray, p1, p2, normalTowardsExit);
				if (uv.x >= 0 && uv.y >= 0 && uv.y <= 1) {
					{
						intersectUVResult.uv = uv;
						intersectUVResult.p1 = p1;
						intersectUVResult.p2 = p2;
						intersectUVResult.normalTowardsExit = normalTowardsExit;
					}
					return true;
				}
			}

			return false;
		}

		bool intersect(const Ray_<T>& ray, glm::tvec2<T>& point)
		{
			IntersectUVResult result;
			if (!this->intersectUV(ray, result)) {
				return false;
			}
			point = ray.s + ray.t * result.uv.u;
			return true;
		}

		shared_ptr<Ray_<T>>
			interact(const Ray_<T>& incoming) {
			IntersectUVResult result;
			if (!this->intersectUV(incoming, result)) {
				return nullptr;
			}

			auto newRay = make_shared<Ray_<T>>();
			{
				newRay->s = incoming.s + result.uv.x * incoming.t;
				newRay->t = ofxCeres::VectorMath::refract(incoming.t
					, result.normalTowardsExit
					, this->exitVsEntranceIOR);
			}

			return newRay;
		}

		template<typename T2>
		MeshBoundary_<T2> castTo()
		{
			MeshBoundary_<T2> instance2;
			{
				for (const auto& vertex : this->vertices) {
					instance2.vertices.emplace(vertex->first, (T2)vertex->second);
				}
				
				// Base
				{
					instance2.center = (glm::tvec2<T2>) this->center;
					instance2.entranceIOR = (T2)this->entranceIOR;
					instance2.exitIOR = (T2)this->exitIOR;
				}
			}
			return instance2;
		}
	};

	typedef MeshBoundary_<float> MeshBoundary;
}