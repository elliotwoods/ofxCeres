#pragma once

struct DistortedMovingHeadError {
	DistortedMovingHeadError(const glm::tvec3<double> & targetPoint, const glm::tvec2<double> & panTiltValuesSignal)
		: targetPoint(targetPoint)
		, panTiltValuesSignal(panTiltValuesSignal) {}

	template <typename T>
	bool operator()(const T * const transformParameters
		, const T * const panDistortionParameters
		, const T * const tiltDistortionParameters
		, T * residuals) const {

		//--
		//Extract parameters
		//
		glm::tvec3<T> translation(transformParameters[0], transformParameters[1], transformParameters[2]);
		glm::tvec3<T> rotationVector(transformParameters[3], transformParameters[4], transformParameters[5]);
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
		

		// Get ideal angles
		auto idealAnglesForTarget = ofxCeres::VectorMath::getPanTiltToTargetInObjectSpace(targetInViewSpace);
		glm::tvec2<T> idealAnglesFromCapture{
			ofxCeres::VectorMath::powerSeries2((T)this->panTiltValuesSignal.x, panDistortionParameters)
			, ofxCeres::VectorMath::powerSeries2((T)this->panTiltValuesSignal.y, tiltDistortionParameters)
		};

		
		//Get the disparity between the real and actual object space rays
		auto angleBetweenResults = ofxCeres::VectorMath::sphericalPolarDistance(idealAnglesForTarget, idealAnglesFromCapture);

		residuals[0] = angleBetweenResults * angleBetweenResults;

		return true;
	}

	static ceres::CostFunction * Create(const glm::tvec3<double> & targetPoint, const glm::tvec2<double> & panTiltValues) {
		return new ceres::AutoDiffCostFunction<DistortedMovingHeadError, 1, 7, 3, 3>(
			new DistortedMovingHeadError(targetPoint, panTiltValues)
			);
	}

	const glm::tvec3<double> targetPoint;
	const glm::tvec2<double> panTiltValuesSignal;
};
