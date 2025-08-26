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
				auto distance = VectorMath::dot(position - this->center, this->normal);
				return position - (T) 2 * distance * this->normal;
			}

			Ray<T>
				reflect(const Ray<T>& ray) const
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

			Ray<T>
				refract(const Ray<T>& incidentRay, T refractiveIndexIn, T refractiveIndexOut) const
			{
				// Find the position where the incident ray intersects the plane (we presume this is 'forwards' on the ray)
				glm::tvec3<T> intersectionPoint = this->intersect(incidentRay);

				// Refract the ray at the intersection point
				refractedTransmission = VectorMath::refract(incidentRay.t
					, -this->normal
					, refractiveIndexOut / refractiveIndexIn);

				// Create a new ray starting from the intersection point
				return TRay<T>(intersectionPoint, refractedTransmission);
			}
		};
	}
}

