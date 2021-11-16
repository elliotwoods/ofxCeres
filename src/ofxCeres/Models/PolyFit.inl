#include "pch_ofxCeres.h"
#include "PolyFit.h"
#include "ofxCeres/VectorMath/VectorMath.h"

template<size_t Order>
struct PolyFitError {
	PolyFitError(const double& x, const double& y)
		: x(x)
		, y(y){}

	template <typename T>
	bool operator()(const T* const coefficients
		, T* residuals) const {

		T result = (T)0;
		double xx = 1;
		for (size_t i = 0; i < Order; i++) {
			result += xx * coefficients[i];
			xx *= x;
		}
		residuals[0] = result - this->y;
		return true;
	}

	static ceres::CostFunction* Create(const double& x, const double& y) {
		return new ceres::AutoDiffCostFunction<PolyFitError<Order>, 1, Order>(
			new PolyFitError(x, y)
			);
	}

	const double x;
	const double y;
};