#pragma once

#include "ofVectorMath.h"
#include "../Exception.h"
#include <utility>

namespace ofxCeres {
	namespace VectorMath {
		//----------
		template<typename T>
		T dot(const glm::tvec2<T>& A, const glm::tvec2<T>& B) {
			return A.x * B.x
				+ A.y * B.y;
		}

		//----------
		template<typename T>
		T dot(const glm::tvec3<T>& A, const glm::tvec3<T>& B) {
			return A.x * B.x
				+ A.y * B.y
				+ A.z * B.z;
		}

		//----------
		template<typename T>
		T length2(const glm::tvec2<T>& vector) {
			return dot(vector, vector);
		}

		//----------
		template<typename T>
		T length2(const glm::tvec3<T>& vector) {
			return dot(vector, vector);
		}

		//----------
		template<typename T>
		T length(const glm::tvec2<T>& vector) {
			return sqrt(length2(vector));
		}

		//----------
		template<typename T>
		T length(const glm::tvec3<T>& vector) {
			return sqrt(length2(vector));
		}

		//----------
		template<typename T>
		T distance2(const glm::tvec3<T>& A, const glm::tvec3<T>& B) {
			auto delta = B - A;
			return dot(delta, delta);
		}

		//----------
		template<typename T>
		T distance(const glm::tvec3<T>& A, const glm::tvec3<T>& B) {
			return sqrt(distance2(A, B));
		}

		//----------
		template<typename T>
		T distance2(const glm::tvec2<T>& A, const glm::tvec2<T>& B) {
			const glm::tvec2<T> delta(B.x - A.x, B.y - A.y);
			return dot(delta, delta);
		}

		//----------
		template<typename T>
		T distance(const glm::tvec2<T>& A, const glm::tvec2<T>& B) {
			return sqrt(distance2(A, B));
		}

		//----------
		template<typename T>
		glm::tvec3<T> normalize(const glm::tvec3<T>& vector) {
			return vector / length(vector);
		}

		//----------
		template<typename T>
		glm::tvec3<T> cross(const glm::tvec3<T>& x, const glm::tvec3<T>& y) {
			return glm::tvec3<T>(
				x.y * y.z - y.y * x.z,
				x.z * y.x - y.z * x.x,
				x.x * y.y - y.x * x.y);
		}

		//----------
		template<typename T>
		glm::tquat<T> eulerToQuat(const glm::tvec3<T>& eulerAngles) {
			glm::tvec3<T> c(cos(eulerAngles[0] * T(0.5)), cos(eulerAngles[1] * T(0.5)), cos(eulerAngles[2] * T(0.5)));
			glm::tvec3<T> s(sin(eulerAngles[0] * T(0.5)), sin(eulerAngles[1] * T(0.5)), sin(eulerAngles[2] * T(0.5)));

			return glm::tquat<T> {
				c.x* c.y* c.z + s.x * s.y * s.z
					, s.x* c.y* c.z - c.x * s.y * s.z
					, c.x* s.y* c.z + s.x * c.y * s.z
					, c.x* c.y* s.z - s.x * s.y * c.z
			};
		}

		//----------
		template<typename T>
		glm::tmat4x4<T> createTransform(const glm::tvec3<T>& translation, const glm::tvec3<T>& rotationVector)
		{
			auto rotationQuat = eulerToQuat(rotationVector);
			auto rotationMat = glm::tmat4x4<T>(rotationQuat);
			return glm::translate(translation) * rotationMat;
		}

		//----------
		template<typename T>
		glm::tvec2<T> getPanTiltToTargetInObjectSpace(const glm::tvec3<T>& objectSpacePoint) {

			// Left (+) corresponds with positive atan, so we switch since professional fixtures are Right (+)
			auto pan = -atan2(objectSpacePoint.x, objectSpacePoint.z) * (T)RAD_TO_DEG;

			T distance2 = objectSpacePoint.x * objectSpacePoint.x
				+ objectSpacePoint.y * objectSpacePoint.y
				+ objectSpacePoint.z * objectSpacePoint.z;

			T distance = sqrt(distance2);

			auto tilt = acos(objectSpacePoint.y / distance) * (T)RAD_TO_DEG;

			return glm::tvec2<T>(pan, tilt);
		}

		//----------
		template<typename T>
		glm::tvec3<T> getObjectSpaceRayForPanTilt(const glm::tvec2<T>& panTiltAngle) {
			auto actualTilt = panTiltAngle.y;

			// Left (+) corresponds with positive atan, so we switch since professional fixtures are Right (+)
			glm::tvec3<T> transmission(
				sin(actualTilt * DEG_TO_RAD) * sin(-panTiltAngle.x * DEG_TO_RAD)
				, cos(actualTilt * DEG_TO_RAD)
				, sin(actualTilt * DEG_TO_RAD) * cos(-panTiltAngle.x * DEG_TO_RAD)
			);

			return transmission;
		}

		//----------
		template<typename T>
		T powerSeries2(const T& x, const T* const coefficients) {
			return x * x * coefficients[0]
				+ x * coefficients[1]
				+ coefficients[2];
		}

		//----------
		// from Wolfram alpha 'y = a*x*x + b * x + c, solve for x'
		template<typename T>
		std::pair<T, T> powerSeries2Inverse(const T& y, const T* const coefficients) {
			auto& a = coefficients[0];
			auto& b = coefficients[1];
			auto& c = coefficients[2];

			if (a != 0) {
				auto solution_1 = -(sqrt(-4 * a * c + 4 * a * y + b * b) + b) / (2 * a);
				auto solution_2 = (sqrt(4 * a * (y - c) + b * b) - b) / (2 * a);

				return { solution_1, solution_2 };
			}
			else if (b != 0) {
				auto solution = (y - c) / b;
				return { solution, solution };
			}
			else {
				throw(ofxCeres::Exception("Cannot invert polynomial"));
			}
		}

		//----------
		template<typename T>
		T sphericalPolarDistance(const glm::tvec2<T>& panTilt1, const glm::tvec2<T>& panTilt2) {
			auto projected1 = getObjectSpaceRayForPanTilt(panTilt1);
			auto projected2 = getObjectSpaceRayForPanTilt(panTilt2);

			auto dotProduct = dot(projected1, projected2);
			auto angleBetweenResults = acos(dotProduct / (length(projected1) * length(projected2)));

			return angleBetweenResults;
		}

		//----------
		template<typename T>
		T pickClosest(const T& target, const T& option1, const T& option2) {
			if (option1 == option1) {
				if (option2 == option2) {
					if (abs(target - option1) < abs(target - option2)) {
						return option1;
					}
					else {
						return option2;
					}
				}
				else {
					return option1;
				}
			}
			else {
				return option2;
			}
		}

		//----------
		// Also page 20,21 of ReMarkable notes May 2021
		template<typename T>
		glm::tvec3<T> reflect(const glm::tvec3<T>& point
			, const glm::tvec3<T>& planeNormal
			, const T& planeD) {

			// create a ray from the point with direction of the plane normal
			const glm::tvec3<T>& rayS = point;
			const glm::tvec3<T>& rayT = planeNormal;

			// intersect the ray and the plane
			// page 20 of ReMarkable notes May 2021
			const T u = -(planeD + dot(rayS, planeNormal))
				/ dot(rayT, planeNormal);
			auto intersection = rayS + u * rayT;

			// follow the ray through the plane by 2x distance to get the reflection
			return rayS + u * (T)2 * rayT;
		}

		//----------
		// Notes ReMarkable November 2022 Page 5
		template<typename T>
		glm::tvec2<T> refract(const glm::tvec2<T>& incident
			, const glm::tvec2<T>& normalTowardsExit
			, const T& exitIORvsIncidentIOR)
		{
			const auto incidentDotNormal = dot(incident, normalTowardsExit);
			const auto theta1 = acos(incidentDotNormal);
			const auto sin_theta1 = sin(theta1);
			const auto sin_theta2 = sin_theta1 / exitIORvsIncidentIOR;
			const auto theta2 = asin(sin_theta2);

			const auto p = normalize(incident - incidentDotNormal * normalTowardsExit);

			return p * sin_theta2 + normalTowardsExit * cos(theta2);
		}

		//----------
		// As defined https://registry.khronos.org/OpenGL-Refpages/gl4/html/refract.xhtml
		template<typename T>
		glm::tvec3<T> refract(const glm::tvec3<T>& incident
			, const glm::tvec3<T>& normalTowardsExit
			, const T& exitIORvsIncidentIOR)
		{
			const auto& I = incident;
			const auto N = -normalTowardsExit;
			const auto& eta = (T)1.0 / exitIORvsIncidentIOR;

			auto k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
			if (k < (T)0.0) {
				return { 0, 0, 0 };
			}
			else {
				return eta * I - (eta * dot(N, I) + sqrt(k)) * N;
			}
		}

		//----------
		template<typename T>
		glm::tvec3<T> applyTransform(const glm::tmat4x4<T>& matrix, const glm::tvec3<T>& vector)
		{
			glm::tvec4<T> vec4(vector.x, vector.y, vector.z, (T)1.0f);
			auto result4 = matrix * vec4;
			result4 /= result4.w;
			return glm::tvec3<T>(result4.x, result4.y, result4.z);
		}

		//----------
		template<typename T>
		glm::tvec3<T> applyRotationOnly(const glm::tmat4x4<T>& matrix, const glm::tvec3<T>& vector)
		{
			auto result = ((glm::tmat3x3<T>) matrix) * vector;
			return result;
		}

		//----------
		// from ofxRay::Ray::distanceTo
		template<typename T>
		T distanceRayToPoint(const glm::tvec3<T> & s, const glm::tvec3<T> & t, const glm::tvec3<T> & point)
		{
			return length(cross(point - s, point - (s + t))) / length(t);
		}

		//----------
		template<typename T>
		T distanceLineToPoint(const glm::tvec3<T>& abc, const glm::tvec2<T>& point)
		{
			auto& a = abc[0];
			auto& b = abc[1];
			auto& c = abc[2];
			return abs(a * point.x + b * point.y + c) / sqrt(a * a + b * b);
		}

		//----------
		// reference : https://blog.demofox.org/2013/10/12/converting-to-and-from-polar-spherical-coordinates-made-easy/
		// but modified
		template<typename T>
		glm::tvec3<T> cartesianToPolar(const glm::tvec3<T>& xyz)
		{
			auto r = length(xyz);
			return {
				r
				, atan2(xyz.y, xyz.x)
				, asin(xyz.z / r)
			};
		}

		//----------
		template<typename T>
		glm::tvec3<T> polarToCartesian(const glm::tvec3<T>& rThetaThi)
		{
			return {
				rThetaThi[0] * cos(rThetaThi[2]) * cos(rThetaThi[1])
				, rThetaThi[0] * cos(rThetaThi[2]) * sin(rThetaThi[1])
				, rThetaThi[0] * sin(rThetaThi[2])
			};
		}

		//----------
		// From GLM implementation
		template<typename T>
		glm::tquat<T> angleAxis(T const& angle, glm::tvec3<T> const& v)
		{
			T const a(angle);
			T const s = sin(a * static_cast<T>(0.5));

			return glm::tquat<T>(cos(a * static_cast<T>(0.5)), v * s);
		}
	}
}
