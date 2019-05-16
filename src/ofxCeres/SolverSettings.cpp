#include "SolverSettings.h"

namespace ofxCeres {
	//----------
	SolverSettings::SolverSettings() {
		this->options.linear_solver_type = ceres::DENSE_QR;
		this->options.minimizer_progress_to_stdout = true;
		this->options.max_num_iterations = 100;
		this->options.function_tolerance = 5e-6;
	}
}