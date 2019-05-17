#pragma once

#define GLM_FORCE_UNRESTRICTED_GENTYPE
#include "ofVectorMath.h"

#include <utility>

namespace ofxCeres {
	namespace VectorMath {
		//----------
		template<typename T>
		T dot(const glm::tvec3<T> & A, const glm::tvec3<T> & B) {
			return A.x * B.x
				+ A.y * B.y
				+ A.z * B.z;
		}

		//----------
		template<typename T>
		T length(const glm::tvec3<T> & vector) {
			return sqrt(dot(vector, vector));
		}

		//----------
		template<typename T>
		glm::tvec3<T> normalize(const glm::tvec3<T> & vector) {
			return vector / length(vector);
		}

		//----------
		template<typename T>
		glm::tquat<T> eulerToQuat(const glm::tvec3<T> & eulerAngles) {
			glm::tvec3<T> c(cos(eulerAngles[0] * T(0.5)), cos(eulerAngles[1] * T(0.5)), cos(eulerAngles[2] * T(0.5)));
			glm::tvec3<T> s(sin(eulerAngles[0] * T(0.5)), sin(eulerAngles[1] * T(0.5)), sin(eulerAngles[2] * T(0.5)));

			glm::tquat<T> result;

			result.w = c.x * c.y * c.z + s.x * s.y * s.z;
			result.x = s.x * c.y * c.z - c.x * s.y * s.z;
			result.y = c.x * s.y * c.z + s.x * c.y * s.z;
			result.z = c.x * c.y * s.z - s.x * s.y * c.z;

			return result;
		}

		//----------
		template<typename T>
		glm::tmat4x4<T> createTransform(const glm::tvec3<T> & translation, const glm::tvec3<T> & rotationVector)
		{
			auto rotationQuat = eulerToQuat(rotationVector);
			auto rotationMat = glm::tmat4x4<T>(rotationQuat);
			return glm::translate(translation) * rotationMat;
		}

		//----------
		template<typename T>
		glm::tvec2<T> getPanTiltToTargetInObjectSpace(const glm::tvec3<T> & objectSpacePoint
			, T tiltOffset = (T) 0.0) {

			// Left (+) corresponds with positive atan, so we switch since professional fixtures are Right (+)
			auto pan = - atan2(objectSpacePoint.x, objectSpacePoint.z) * (T) RAD_TO_DEG;

			T distance2 = objectSpacePoint.x * objectSpacePoint.x
				+ objectSpacePoint.y * objectSpacePoint.y
				+ objectSpacePoint.z * objectSpacePoint.z;

			T distance = sqrt(distance2);

			auto actualTilt = acos(objectSpacePoint.y / distance) * (T)RAD_TO_DEG;

			auto tiltCommand = actualTilt - tiltOffset;

			return glm::tvec2<T>(pan, tiltCommand);
		}

		//----------
		template<typename T>
		glm::tvec3<T> getObjectSpaceRayForPanTilt(const glm::tvec2<T> & panTiltAngle, T tiltOffset) {
			auto actualTilt = panTiltAngle.y + tiltOffset;

			// Left (+) corresponds with positive atan, so we switch since professional fixtures are Right (+)
			glm::tvec3<T> transmission(
				sin(actualTilt * DEG_TO_RAD) * sin(- panTiltAngle.x * DEG_TO_RAD)
				, cos(actualTilt * DEG_TO_RAD)
				, sin(actualTilt * DEG_TO_RAD) * cos(- panTiltAngle.x * DEG_TO_RAD)
			);

			return transmission;
		}

		//----------
		template<typename T>
		T powerSeries2(const T & x, const T * const coefficients) {
			return x * x * coefficients[0]
				+ x * coefficients[1]
				+ coefficients[2];
		}

		//----------
		// from Wolfram alpha 'inverse of x*x*a + a*b + c'
		template<typename T>
		std::pair<T, T> powerSeries2Inverse(const T & x, const T * const coefficients) {
			auto sqrt_inner = (T)4 * coefficients[0] * x
				+ coefficients[1] * coefficients[1]
				- (T)4 * coefficients[2] * x;
			if (sqrt_inner < (T)0) {
				throw(ofxCeres::Exception("No root found"));
			}

			auto sqrt_part = sqrt(sqrt_inner);

			T solution_1 = -coefficients[1] - sqrt_part;
			solution_1 /= (T)2 * x;

			T solution_2 = -coefficients[1] + sqrt_part;
			solution_2 /= (T)2 * x;

			return { solution_1, solution_2 };
		}

		//----------
		template<typename T>
		T sphericalPolarDistance(const glm::tvec2<T> & panTilt1, const glm::tvec2<T> & panTilt2) {
			auto projected1 = getObjectSpaceRayForPanTilt(panTilt1, (T) 0.0);
			auto projected2 = getObjectSpaceRayForPanTilt(panTilt2, (T) 0.0);

			auto dotProduct = dot(projected1, projected2);
			auto angleBetweenResults = acos(dotProduct / (length(projected1) * length(projected2)));

			return angleBetweenResults;
		}

		//----------
		template<typename T>
		T pickClosest(const T & target, const T & option1, const T & option2) {
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
	}
}