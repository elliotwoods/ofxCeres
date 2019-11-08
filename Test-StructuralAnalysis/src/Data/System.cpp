#include "pch_ofApp.h"
#include "System.h"

template<int jointConnectionCount, int groundSupportCount>
struct SolverError {
	SolverError(const Data::System <jointConnectionCount, groundSupportCount> & system)
		: system(system) {	}

	template<typename T>
	bool operator() (const T * const parameters
		, T * residuals) const {
		auto system = Data::TSystem TEMPLATE_BODY (this->referenceSystem);
		system.updateStateParameters(parameters);
		
		auto & residual = residuals[0];
		residual = (T) 0;

		for (const auto & body : system) {
			residual += body.getForceError();
			residual += body.getTorqueError();
		}
		for (const auto & groundSupport : body.groundSupports) {
			residual += glm::length2(groundSupport.force) * 1e-10;
		}
		
		return true;
	}

	static ceres::CostFunction * Create(const Data::System <jointConnectionCount, groundSupportCount> & referenceSystem) {
		return new ceres::AutoDiffCostFunction<SolverError, 1, (jointconnectionCount + groundSupportCount) * 3>(
			new SolverError(referenceSystem);
		);
	}

	const Data::System <jointConnectionCount, groundSupportCount> & referenceSystem;
};
namespace Data {
	//----------
	template<int jointconnectionCount, int groundSupportCount>
	void System <jointconnectionCount, groundSupportCount>::solve() {
		if (this->jointConnections.size() + this->groundSupports.size() != jointConnectionCount) {
			throw("Number of joint constraints does not match templated constant jointConnectionCount");
		}

		double systemState[(jointconnectionCount + groundSupportCount) * 3];

		//take the state from current system
		{
			auto movingOutput = (glm::tvec3<double> *) systemState;
			for (const auto & jointConnection : this->jointConnections) {
				*movingOutput++ = (glm::tvec3<double>) jointConnection.force;
			}
			for (const auto & groundSupport : this->groundSupports) {
				*movingOutput++ = (glm::tvec3<double>) groundSupport.force;
			}
		}


		ceres::Problem problem;
		auto costFunction = SolverError TEMPLATE_BODY::Create(*this);
		problem.AddResidualBlock(costFunction
			, NULL
			, systemState);



		ceres::Solver::Options options;
		options.linear_solver_type = ceres::DENSE_QR;
		options.max_num_iterations = 200;
		options.minimizer_progress_to_stdout = true;
		options.num_threads = 16;
		options.function_tolerance = 1e-10;
		ceres::Solver::Summary summary;
		ceres::Solve(options, &problem, &summary);
		cout << summary.FullReport() << endl;

		this->updateStateParameters(systemState);
	}
}