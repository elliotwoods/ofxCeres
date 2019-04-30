#pragma once

#define GLM_FORCE_UNRESTRICTED_GENTYPE
#include "ofVectorMath.h"

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
		glm::tvec2<T> getPanTiltToTargetInObjectSpace(const glm::tvec3<T> & objectSpacePoint, T tiltOffset) {
			auto pan = atan2(objectSpacePoint.x, objectSpacePoint.z) * (T) RAD_TO_DEG;

			T distance = objectSpacePoint.x * objectSpacePoint.x
				+ objectSpacePoint.y * objectSpacePoint.y
				+ objectSpacePoint.z * objectSpacePoint.z;

			auto tilt = acos(objectSpacePoint.y / distance) * (T) RAD_TO_DEG - tiltOffset;

			return glm::tvec2<T>(pan, tilt);
		}

		//----------
		template<typename T>
		glm::tvec3<T> getObjectSpaceRayForPanTilt(const glm::tvec2<T> & panTiltAngle, T tiltOffset) {
			auto actualTilt = panTiltAngle.y + tiltOffset;

			glm::tvec3<T> transmission(
				sin(actualTilt * DEG_TO_RAD) * sin(panTiltAngle.x * DEG_TO_RAD)
				, cos(actualTilt * DEG_TO_RAD)
				, sin(actualTilt * DEG_TO_RAD) * cos(panTiltAngle.x * DEG_TO_RAD)
			);

			return transmission;
		}
	}
}