#pragma once

#include "ofxCeres.h"
#include "../Data/StewartPlatform.h"

namespace Solvers {
	class StewartPlatformFK : ofxCeres::Models::Base
	{
	public:
		struct Solution
		{
			glm::vec3 rotationVector;
			glm::vec3 translation;
		};

		static ofxCeres::SolverSettings defaultSolverSettings();

		typedef ofxCeres::Result<Solution> Result;

		static Result solve(Data::StewartPlatform&
			, bool useExistingSolution = true
			, const ofxCeres::SolverSettings& solverSettings = defaultSolverSettings());
	};
}
