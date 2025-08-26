#pragma once

#include <string>
#include <ceres/ceres.h>
#include "Exception.h"

namespace ofxCeres {
	template<typename SolutionType, typename ResidualType = float>
	struct Result {
		Result(const ceres::Solver::Summary& summary)
		: summary(summary) {
			this->residual = summary.final_cost;
		}

		Result(const ceres::Solver::Summary& summary, double residual)
			: summary(summary) {
			this->residual = residual;
		}

		Result(const ofxCeres::Exception& exception) {
			this->isError = true;
			this->errorMessage = string(exception.what());
		}

		Result(bool success) { // Use this if you're making your own result
			if (success) {
				this->isError = false;
				this->summary.termination_type == ceres::TerminationType::CONVERGENCE;
			}
			else {
				this->isError = true;
			}
		}

		bool isConverged() const {
			if (this->isError) {
				return false;
			}
			else {
				return this->summary.termination_type == ceres::TerminationType::CONVERGENCE;
			}
		}
		ceres::Solver::Summary summary;
		
		SolutionType solution;
		double residual = 0.0;

		bool isError = false;
		std::string errorMessage;
	};
}