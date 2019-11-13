#include "pch_ofxCeres.h"
#include "ofPolyline.h"

template<int stateCount> 
struct IKSolverError {
	IKSolverError(const ofxCeres::Models::InverseKinematics::System & system)
		: system(system) {

	}

	template<typename T>
	bool operator()(const T * const parameters
		, T * residuals) const {
		auto & residual = residuals[0];
		residual = (T) 0.0;

		auto PandRs = system.getPandRsForState<T>(parameters);
		for (const auto & jointPositionConstraint : this->system.jointPositionConstraints) {
			residual += jointPositionConstraint.getDistance(PandRs);
		}
		for (const auto & jointAngleConstraint: this->system.jointAngleConstraints) {
			residual += jointAngleConstraint.getDistance(PandRs);
		}
		for (const auto & domainConstraint : this->system.domainConstraints) {
			residual += domainConstraint.getDistance(PandRs);
		}
		return true;
	}

	static ceres::CostFunction * Create(const ofxCeres::Models::InverseKinematics::System & system) {
		return new ceres::AutoDiffCostFunction<IKSolverError, 1, stateCount>(
			new IKSolverError(system)
		);
	}

	const ofxCeres::Models::InverseKinematics::System & system;
};

namespace ofxCeres {
	namespace Models {
		namespace InverseKinematics {
			//-----------
			template<int stateCount>
			bool System::solve(const SolverSettings & solverSettings) {
				//check sizes
				if (stateCount > this->bodyLengths.size()) {
					throw(ofxCeres::Exception("bodyLengths doesn't match hard-coded stateCount"));
				}

				//initialise the parameters if required
				if (this->currentRotationState.size() != stateCount) {
					this->currentRotationState.assign(stateCount, 0.0);
				}

				ceres::Problem problem;
				auto costFunction = IKSolverError<stateCount>::Create(*this);
				problem.AddResidualBlock(costFunction
					, NULL
					, this->currentRotationState.data());

				ceres::Solver::Summary summary;

				//Annealing adds noise if we solved but dont meet the threshold
				for (int i = 0; i < this->annealing.maxIterations; i++) {
					ceres::Solve(solverSettings.options, &problem, &summary);
					if (summary.termination_type == ceres::CONVERGENCE) {
						if (summary.final_cost < this->annealing.finalCostTarget) {
							if (solverSettings.printReport) {
								cout << summary.FullReport() << endl;
							}
							return true;
						}
						else {
							for (int i = 0; i < stateCount; i++) {
								this->currentRotationState[i] += ofRandomf() * this->annealing.noise;
							}
						}
					}
				}

				if (solverSettings.printReport) {
					cout << summary.FullReport() << endl;
				}
				return false;
			}
		}
	}
}
