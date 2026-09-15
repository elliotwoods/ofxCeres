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


			template<typename T2>
			Plane<T2>
				castTo()
			{
				Plane<T2> castPlane;
				{
					castPlane.center = (glm::tvec3<T2>) this->center;
					castPlane.normal = (glm::tvec3<T2>) this->normal;
				}
				return castPlane;
			}


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
				const auto& A = ABCD[0];
				const auto& B = ABCD[1];
				const auto& C = ABCD[2];
				const auto& D = ABCD[3];

				glm::tvec3<T> direction(A, B, C);

				auto u = -D / VectorMath::dot(direction, direction);
				this->center = u * direction;
				this->normal = glm::normalize(direction);
			}

			Plane<T>
				transform(const glm::tmat4x4<T>& transform) const
			{
				Plane<T> out;

				// center transforms as a point
				const glm::tvec4<T> c4 = transform * glm::tvec4<T>(this->center, T(1));
				out.center = glm::tvec3<T>(c4);

				// normal transforms by rotation only (top-left 3x3); re-normalize for safety
				const glm::tmat3x3<T> R(transform);
				out.normal = glm::normalize(R * this->normal);

				return out;
			}

			T
				distanceTo(const glm::tvec3<T>& point) const
			{
				// Signed distance from point to plane through `center` with `normal`
				// If normal isn't unit length, divide by its magnitude.
				const T nlen = VectorMath::length(this->normal);

				// We presume nlen > 0

				return VectorMath::dot(this->normal, point - this->center) / nlen;
			}

			glm::tvec3<T>
				intersect(const Ray<T>& ray) const
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
				reflect(const glm::tvec3<T>& position) const
			{
				// Ensure unit normal for correct reflection distance
				const glm::tvec3<T> n = glm::normalize(this->normal);
				const T signedDistance = glm::dot(position - this->center, n);
				return position - T(2) * signedDistance * n;
			}

			Ray<T>
				reflect(const Ray<T>& ray) const
			{
				// Unit normal
				const glm::tvec3<T> n = glm::normalize(this->normal);

				// Ray-plane intersection along ray: s + t * d
				// denom ~ 0 => parallel (no reflection event)
				const T denom = glm::dot(ray.t, n);
				const T eps = std::numeric_limits<T>::epsilon() * T(32);

				// Parallel or nearly-parallel => no valid reflection
				if (glm::abs(denom) <= eps) {
					return ray; // or return a sentinel/invalid ray if your API supports it
				}

				// Solve for hit parameter t: dot(center - s, n) / dot(d, n)
				const T tHit = glm::dot(this->center - ray.s, n) / denom;

				// If intersection is behind the ray start, treat as no reflection
				if (tHit < T(0)) {
					return ray; // or a sentinel/invalid ray
				}

				// Hit point on plane
				const glm::tvec3<T> hitPoint = ray.s + tHit * ray.t;

				// Reflect direction: d' = d - 2(d·n)n   (n must be unit)
				const glm::tvec3<T> reflectedDir = ray.t - T(2) * glm::dot(ray.t, n) * n;

				// Build reflected ray starting at the hit point
				Ray<T> reflectedRay;
				reflectedRay.s = hitPoint;
				reflectedRay.t = reflectedDir;       // same magnitude as input; specular reflection preserves |t|

				// If you prefer keeping the original length exactly (in case of tiny numeric drift), you can re-scale:
				// reflectedRay.setLength(VectorMath::length(ray.t));

				return reflectedRay;
			}

			Ray<T>
				refract(const Ray<T>& incidentRay, T refractiveIndexIn, T refractiveIndexOut) const
			{
				// Find the position where the incident ray intersects the plane (we presume this is 'forwards' on the ray)
				glm::tvec3<T> intersectionPoint = this->intersect(incidentRay);

				// Refract the ray at the intersection point
				auto refractedTransmission = VectorMath::refract(incidentRay.t
					, -this->normal
					, refractiveIndexOut / refractiveIndexIn);

				// Create a new ray starting from the intersection point
				return Ray<T>(intersectionPoint, refractedTransmission);
			}
		};
	}
}

