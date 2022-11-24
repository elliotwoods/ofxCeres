#pragma once

#include "../Models/OpticalSystem.h"

namespace Solvers {
	class OpticalSystemSolver {
	public:
		struct Solution {
			Models::OpticalSystem opticalSystem;
		};

		typedef ofxCeres::Result<Solution> Result;

		static ofxCeres::SolverSettings getDefaultSolverSettings();

		static Result solve(const Models::OpticalSystem& initialCondition
			, const vector<Models::Ray>& rays
			, const glm::vec2& target
			, const ofxCeres::SolverSettings& solverSettings);
	};
}