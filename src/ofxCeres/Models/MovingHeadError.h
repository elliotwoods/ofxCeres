#pragma once

struct MovingHeadError {
	MovingHeadError(const glm::tvec3<double> & targetPoint, const glm::tvec2<double> & panTiltValues)
		: targetPoint(targetPoint)
		, panTiltValues(panTiltValues) {}

	template <typename T>
	bool operator()(const T * const translationParameters
		, const T* const rotationParameters
		, const T* const tiltOffsetParameters
		, T * residuals) const {

		//--
		//Extract parameters
		glm::tvec3<T> translation(translationParameters[0], translationParameters[1], translationParameters[2]);
		glm::tvec3<T> rotationVector(rotationParameters[0], rotationParameters[1], rotationParameters[2]);

		T tiltOffset = tiltOffsetParameters[0];

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
		return new ceres::AutoDiffCostFunction<MovingHeadError, 1, 3, 3, 1>(
			new MovingHeadError(targetPoint, panTiltValues)
			);
	}

	const glm::tvec3<double> targetPoint;
	const glm::tvec2<double> panTiltValues;
};