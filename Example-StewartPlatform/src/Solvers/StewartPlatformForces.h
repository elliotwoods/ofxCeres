#pragma once

#include "ofxCeres.h"
#include "../Data/StewartPlatform.h"

namespace Solvers {
	class StewartPlatformForces : ofxCeres::Models::Base
	{
	public:
		struct Solution
		{
			double forces[6]{ 0, 0, 0, 0, 0, 0 };
		};

		static ofxCeres::SolverSettings defaultSolverSettings();

		typedef ofxCeres::Result<Solution> Result;

		static Result solve(Data::StewartPlatform&
			, bool useExistingSolution = true
			, const ofxCeres::SolverSettings& solverSettings = defaultSolverSettings());
	};
}
