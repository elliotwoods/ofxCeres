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
		this->options.num_threads = std::thread::hardware_concurrency();
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
			, this->parameterTolerance
			, this->minimizerType
			, this->loggingType);
	}

	//----------
	ParameterisedSolverSettings::ParameterisedSolverSettings(const ofxCeres::SolverSettings& solverSettings)
		: ParameterisedSolverSettings()
	{
		this->printReport = solverSettings.printReport;
		this->printEachStep = solverSettings.options.minimizer_progress_to_stdout;
		this->maxIterations = solverSettings.options.max_num_iterations;
		this->functionTolerance = solverSettings.options.function_tolerance;
		this->gradientTolerance = solverSettings.options.gradient_tolerance;
		this->parameterTolerance = solverSettings.options.parameter_tolerance;
		this->minimizerType = (int)solverSettings.options.minimizer_type;
		this->loggingType = (int)solverSettings.options.logging_type;
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
		solverSettings.options.minimizer_type = (ceres::MinimizerType)this->minimizerType.get();
		solverSettings.options.logging_type = (ceres::LoggingType)this->loggingType.get();

		solverSettings.options.num_threads = std::thread::hardware_concurrency();

		return solverSettings;
	}
}