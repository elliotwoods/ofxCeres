#pragma once

#include <ceres/ceres.h>
#include "ofParameter.h"

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

	class ParameterisedSolverSettings : public ofParameterGroup {
	public:
		ParameterisedSolverSettings();
		ParameterisedSolverSettings(const ofxCeres::SolverSettings&);

		SolverSettings getSolverSettings();

		ofParameter<bool> printReport{ "Print report", true };
		ofParameter<bool> printEachStep{ "Print each step", true };
		ofParameter<int> maxIterations{ "Max iterations", 100 };
		ofParameter<float> functionTolerance{ "Function tolerance", 1e-6 };
		ofParameter<float> gradientTolerance{ "Gradient tolerance", 1e-10 };
		ofParameter<float> parameterTolerance{ "Parameter tolerance", 1e-8 };
		ofParameter<int> minimizerType{ "Minimizer type", 1 };
		ofParameter<int> loggingType{ "Logging type", 1 };
	};
}