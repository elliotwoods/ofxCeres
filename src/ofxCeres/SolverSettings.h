#pragma once

#include <ceres/ceres.h>

namespace ofxCeres {
	struct SolverSettings {
		SolverSettings();
		ceres::Solver::Options options;

#ifdef _DEBUG
		bool printReport = true;
#else
		bool printReport = false;
#endif
	};
}