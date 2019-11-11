#include "pch_ofApp.h"
#include "System.h"
#define STATE_COUNT 12
#define CONSTRAINT_COUNT 2

#include "ofPolyline.h"

TEMPLATE_HEAD 
struct IKSolverError {
	IKSolverError(const Data::System TEMPLATE_BODY & system)
		: system(system) {

	}

	template<typename T>
	bool operator()(const T * const parameters
		, T * residuals) const {
		auto movingResidual = residuals;
		Data::TPositionAndRotation<T> PandRs[stateCount + 1];
		system.getPandRsForState<T>(parameters, PandRs);
		for (const auto & jointPositionConstraint : this->system.jointPositionConstraints) {
			*movingResidual++ = jointPositionConstraint.getDistance(PandRs);
		}
		for (const auto & jointAngleConstraint: this->system.jointAngleConstraints) {
			*movingResidual++ = jointAngleConstraint.getDistance(PandRs);
		}
		return true;
	}

	static ceres::CostFunction * Create(const Data::System TEMPLATE_BODY & system) {
		return new ceres::AutoDiffCostFunction<IKSolverError, constraintCount, stateCount>(
			new IKSolverError(system)
		);
	}

	const Data::System TEMPLATE_BODY & system;
};
namespace Data {
	//-----------
	TEMPLATE_HEAD
	void System TEMPLATE_BODY ::solve() {
		//check sizes
		if (stateCount > this->bodyLengths.size()) {
			throw("bodyLengths doesn't match hard-coded stateCount");
		}
		if (jointPositionConstraints.size() + jointAngleConstraints.size() > constraintCount) {
			throw("Number of constraints does not match hard-coded value");
		}

		//initialise the parameters if required
		if (this->currentRotationState.size() != stateCount) {
			this->currentRotationState.assign(stateCount, 0.0);
		}

		ceres::Problem problem;
		auto costFunction = IKSolverError TEMPLATE_BODY ::Create(*this);
		problem.AddResidualBlock(costFunction
			, NULL
			, this->currentRotationState.data());

		ceres::Solver::Options options;
		options.linear_solver_type = ceres::DENSE_QR;
		options.max_num_iterations = 200;
		options.minimizer_progress_to_stdout = false;
		options.num_threads = 16;
		options.function_tolerance = 1e-10;
		options.line_search_direction_type = ceres::LBFGS;
		options.minimizer_type = ceres::TRUST_REGION;
		ceres::Solver::Summary summary;
		for (int i = 0; i < this->annealing.maxIterations; i++) {
			ceres::Solve(options, &problem, &summary);
			if (summary.final_cost < this->annealing.threshold) {
				break;
			}
			else {
				for (int i = 0; i < stateCount; i++) {
					this->currentRotationState[i] += ofRandomf() * this->annealing.noise;
				}
			}
		}
		//cout << summary.FullReport() << endl;
	}

	//-----------
	TEMPLATE_HEAD
	void System TEMPLATE_BODY ::getCurrentPositionsAndRotations(PositionAndRotation * output) const {
		this->getPandRsForState(this->currentRotationState.data(), output);
	}

	//-----------
	TEMPLATE_HEAD
	void System TEMPLATE_BODY ::draw() {
		PositionAndRotation PandRs[stateCount + 1];
		this->getCurrentPositionsAndRotations(PandRs);
		ofPolyline line;
		for (int i = 0; i < stateCount + 1; i++) {
			const auto & PandR = PandRs[i];
			line.addVertex(PandR.position.x, PandR.position.y);
		}
		line.draw();
	}
}

template class Data::System<12, 2>;

/*
template class Data::System<1, 1>;
template class Data::System<2, 1>;
template class Data::System<3, 1>;
template class Data::System<4, 1>;
template class Data::System<5, 1>;
template class Data::System<6, 1>;
template class Data::System<7, 1>;
template class Data::System<8, 1>;
template class Data::System<9, 1>;
template class Data::System<10, 1>;
template class Data::System<11, 1>;
template class Data::System<12, 1>;
template class Data::System<13, 1>;
template class Data::System<14, 1>;
template class Data::System<15, 1>;
template class Data::System<16, 1>;
template class Data::System<17, 1>;
template class Data::System<18, 1>;
template class Data::System<19, 1>;
template class Data::System<20, 1>;
template class Data::System<22, 1>;
template class Data::System<23, 1>;
template class Data::System<24, 1>;
template class Data::System<25, 1>;
template class Data::System<26, 1>;
template class Data::System<27, 1>;
template class Data::System<28, 1>;
template class Data::System<29, 1>;
template class Data::System<30, 1>;
template class Data::System<1, 2>;
template class Data::System<2, 2>;
template class Data::System<3, 2>;
template class Data::System<4, 2>;
template class Data::System<5, 2>;
template class Data::System<6, 2>;
template class Data::System<7, 2>;
template class Data::System<8, 2>;
template class Data::System<9, 2>;
template class Data::System<10, 2>;
template class Data::System<11, 2>;
template class Data::System<12, 2>;
template class Data::System<13, 2>;
template class Data::System<14, 2>;
template class Data::System<15, 2>;
template class Data::System<16, 2>;
template class Data::System<17, 2>;
template class Data::System<18, 2>;
template class Data::System<19, 2>;
template class Data::System<20, 2>;
template class Data::System<22, 2>;
template class Data::System<23, 2>;
template class Data::System<24, 2>;
template class Data::System<25, 2>;
template class Data::System<26, 2>;
template class Data::System<27, 2>;
template class Data::System<28, 2>;
template class Data::System<29, 2>;
template class Data::System<30, 2>;
*/
