#pragma once

#include <ceres/ceres.h>

namespace ofxCeres {
	struct SolverSettings {
		SolverSettings();
		ceres::Solver::Options options;

		bool printReport = true;
	};
}