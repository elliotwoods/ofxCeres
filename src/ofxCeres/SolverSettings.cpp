#include "pch_ofxCeres.h"
#include "SolverSettings.h"

namespace ofxCeres {
	//----------
	SolverSettings::SolverSettings()
	{
		this->options.linear_solver_type = ceres::DENSE_QR;
		this->options.minimizer_progress_to_stdout = true;
		this->options.max_num_iterations = 100;
		this->options.function_tolerance = 5e-6;
	}

	//----------
	ParameterisedSolverSettings::ParameterisedSolverSettings()
	{
		this->setName("Solver settings");
		this->add(this->printReport
			, this->printEachStep
			, this->maxIterations
			, this->functionTolerance
			, this->gradientTolerance
			, this->parameterTolerance);
	}

	//----------
	SolverSettings
	ParameterisedSolverSettings::getSolverSettings()
	{
		SolverSettings solverSettings;
		
		solverSettings.printReport = this->printReport.get();
		solverSettings.options.minimizer_progress_to_stdout = this->printEachStep.get();
		solverSettings.options.max_num_iterations = this->maxIterations.get();
		solverSettings.options.function_tolerance = this->functionTolerance.get();
		solverSettings.options.gradient_tolerance = this->gradientTolerance.get();
		solverSettings.options.parameter_tolerance = this->parameterTolerance.get();
		
		solverSettings.options.num_threads = std::thread::hardware_concurrency();

		return solverSettings;
	}
}