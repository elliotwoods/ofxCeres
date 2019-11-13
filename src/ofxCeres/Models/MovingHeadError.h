#pragma once

struct MovingHeadError {
	MovingHeadError(const glm::tvec3<double> & targetPoint, const glm::tvec2<double> & panTiltValues)
		: targetPoint(targetPoint)
		, panTiltValues(panTiltValues) {}

	template <typename T>
	bool operator()(const T * const transformParameters
		, T * residuals) const {

		//--
		//Extract parameters
		glm::tvec3<T> translation(transformParameters[0], transformParameters[1], transformParameters[2]);
		glm::tvec3<T> rotationVector(transformParameters[3], transformParameters[4], transformParameters[5]);

		T tiltOffset = transformParameters[6];

		auto transform = ofxCeres::VectorMath::createTransform(translation, rotationVector);
		//
		//--


		//--
		// Ignore point if it is in same position as the light fixture right now
		if ((glm::tvec3<T>)this->targetPoint == translation) {
			residuals[0] = (T) 0.0;
			return true;
		}
		//
		//--


		//--
		//World -> Object space
		//

		//apply rigid body transform
		glm::tvec4<T> targetInViewSpace4 = glm::inverse(transform) * glm::tvec4<T>(this->targetPoint, 1.0);
		targetInViewSpace4 /= targetInViewSpace4.w;
		auto targetInViewSpace = glm::tvec3<T>(targetInViewSpace4);

		//
		//--

		glm::tvec3<T> rayCastForPanTiltValues = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt<T>(this->panTiltValues, tiltOffset);

		//--
		//Get the disparity between the real and actual object space rays
		//
		auto dotProduct = ofxCeres::VectorMath::dot(rayCastForPanTiltValues, targetInViewSpace);
		dotProduct = dotProduct / (ofxCeres::VectorMath::length(rayCastForPanTiltValues) * ofxCeres::VectorMath::length(targetInViewSpace));
		auto angleBetweenResults = acos(dotProduct);

		residuals[0] = angleBetweenResults * angleBetweenResults;

		return true;
	}

	static ceres::CostFunction * Create(const glm::tvec3<double> & targetPoint, const glm::tvec2<double> & panTiltValues) {
		return new ceres::AutoDiffCostFunction<MovingHeadError, 1, 7>(
			new MovingHeadError(targetPoint, panTiltValues)
			);
	}

	const glm::tvec3<double> targetPoint;
	const glm::tvec2<double> panTiltValues;
};