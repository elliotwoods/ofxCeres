#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

#include "pch_ofxCeres.h"
#include "ofxCeres/VectorMath/VectorMath.h"
#include "Ray.h"

namespace ofxCeres {
	namespace Models {
		template<typename T>
		struct Plane {
			glm::tvec3<T> center;
			glm::tvec3<T> normal;

			glm::tvec4<T>
				getABCD() const
			{
				glm::tvec4<T> ABCD;
				ABCD.x = this->normal.x;
				ABCD.y = this->normal.y;
				ABCD.z = this->normal.z;
				ABCD.w = -VectorMath::dot(this->normal, this->center);
				return ABCD;
			}

			void
				setABCD(const glm::tvec4<T>& ABCD)
			{
				const& A = ABCD[0];
				const& B = ABCD[1];
				const& C = ABCD[2];
				const& D = ABCD[3];

				glm::tvec3<T> direction(A, B, C);

				auto u = -D / VectorMath::dot(direction, direction);
				this->center = u * direction;
				this->normal = glm::normalize(direction);
			}

			T
				distanceTo(const glm::tvec3<T>& point)
			{

			}

			glm::tvec3<T>
				intersect(const Ray<T>& ray)
			{
				if (glm::dot(ray.t, normal) == (T) 0.0f)
				{
					// Plane is parallel to ray
					return { 1000, 1000, 1000 };
				}

				auto u = VectorMath::dot(normal, center - ray.s) / VectorMath::dot(normal, ray.t);
				auto position = ray.s + u * ray.t;
				return position;
			}

			glm::tvec3<T>
				reflect(const glm::tvec3<T>& position)
			{
				auto distance = VectorMath::dot(position - this->center, this->normal);
				return position - (T) 2 * distance * this->normal;
			}

			Ray<T>
				reflect(const Ray<T>& ray)
			{
				// Find where ray hits mirror
				auto positionRayHitsPlane = this->intersect(ray);

				// Reflect point transmitted through mirror point of ray in mirror
				auto reflectedTThroughMirror = this->reflect(positionRayHitsPlane + ray.t);

				// Emit the ray in reverse
				Ray<T> reflectedRay;
				{
					reflectedRay.s = positionRayHitsPlane;
					reflectedRay.setEnd(reflectedTThroughMirror);
				}
				return reflectedRay;
			}
		};
	}
}

