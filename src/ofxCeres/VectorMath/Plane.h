#pragma once

#include "VectorMath.h"
#include "Ray.h"
#include "../Exception.h"

namespace ofxCeres {
	namespace VectorMath {
		//----------
		template<typename T>
		struct TPlane {
			TPlane() = default;
			TPlane(const glm::tvec3<T>& center, const glm::tvec3<T>& normal)
				: center(center), normal(normal) {}

			glm::tvec3<T> center{ 0, 0, 0 };
			glm::tvec3<T> normal{ 0, 1, 0 };

			glm::tvec3<T>
				intersect(const TRay<T>& ray) const {
				// Calculate the denominator
				T denominator = dot(normal, ray.t);
				if (abs(denominator) < (T) 1e-6) {
					throw ofxCeres::Exception("Ray is parallel to the plane");
				}

				// Calculate the numerator
				T d = dot(normal, center - ray.s);
				T u = d / denominator;

				// Calculate the intersection point
				return ray.s + u * ray.t;
			}

			TRay<T>
				refract(const TRay<T>& incidentRay, T refractiveIndexIn, T refractiveIndexOut) const
			{
				// Find the position where the incident ray intersects the plane (we presume this is 'forwards' on the ray)
				glm::tvec3<T> intersectionPoint = intersect(incidentRay);

				// Refract the ray at the intersection point
				refractedTransmission = VectorMath::refract(incidentRay.t
					, -this->normal
					, refractiveIndexOut / refractiveIndexIn);

				// Create a new ray starting from the intersection point
				return TRay<T>(intersectionPoint, refractedTransmission);
			}

			TPlane<T>
				applyTransform(const glm::tmat4x4<T>& transform) const
			{
				auto newCenter = VectorMath::applyTransform(transform, center);
				auto newNormal = glm::normalize(glm::tmat3x3<T>(transform) * normal);
				return TPlane<T> {
					newCenter
					, newNormal
				};
			}
		};
	}
}