#pragma once

#include <string>
#include <ceres/ceres.h>

namespace ofxCeres {
	template<typename SolutionType, typename ResidualType = float>
	struct Result {
		bool success;
		SolutionType solution;
		double residual;

		ceres::Solver::Summary summary;

		std::string errorMessage;
	};
}