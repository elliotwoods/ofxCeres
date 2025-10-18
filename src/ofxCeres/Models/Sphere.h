#pragma once

#include "Ray.h"

namespace ofxCeres {
	namespace Models {
		template<typename T>
		struct Sphere {
			glm::tvec3<T> center;
			T radius;

			bool intersect(const Ray<T>& ray
				, glm::tvec3<T>& forwardPoint
				, glm::tvec3<T>& reversePoint) const
			{
				// Ray: p = origin + t * direction
				// Sphere: |p - center|^2 = radius^2
				// Substitute ray into sphere equation and solve for t:
				// |origin + t*direction - center|^2 = radius^2
				// Let L = origin - center
				// (direction . direction) * t^2 + 2 * (L . direction) * t + (L . L - r^2) = 0

				const glm::tvec3<T> L = ray.s - center;
				const T a = glm::dot(ray.t, ray.t);
				const T b = (T) 2.0 * glm::dot(L, ray.t);
				const T c = glm::dot(L, L) - radius * radius;

				const T discriminant = b * b - (T) 4.0 * a * c;
				if (discriminant < T(0)) {
					return false;
				}

				const T sqrtDisc = sqrt(discriminant);
				const T denom = (T) 2.0 * a;
				T u1 = (-b - sqrtDisc) / denom;
				T u2 = (-b + sqrtDisc) / denom;

				// Ensure t1 <= t2
				if (u1 > u2) std::swap(u1, u2);

				glm::tvec3<T> p1 = ray.s + u1 * ray.t;
				glm::tvec3<T> p2 = ray.s + u2 * ray.t;

				// Forward point is the one with the larger u in the direction of the ray
				if (u2 > u1) {
					forwardPoint = p2;
					reversePoint = p1;
				} else {
					forwardPoint = p1;
					reversePoint = p2;
				}

				return true;
			}
		};
	}
}