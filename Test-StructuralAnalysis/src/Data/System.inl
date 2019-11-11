#include "pch_ofApp.h"
#include "System.h"
#include "DrawProperties.h"

template<int jointConnectionCount, int groundSupportCount>
struct SolverError {
	SolverError(const Data::System & system)
		: referenceSystem(system) {	}

	template<typename T>
	bool operator() (const T * const parameters
		, T * residuals) const {
		auto system = Data::TSystem<T>(this->referenceSystem);
		system.updateStateParameters(parameters);
		
		auto & residual = residuals[0];
		residual = (T) 0;

		for (const auto & bodyIt : system.bodies) {
			residual += bodyIt.second.getForceError();
			residual += bodyIt.second.getTorqueError();
		}
		for (const auto & groundSupport : system.groundSupports) {
			residual += ofxCeres::VectorMath::length2(groundSupport.force) * 1e-8;
		}
		
		return true;
	}

	static ceres::CostFunction * Create(const Data::System & referenceSystem) {
		return new ceres::AutoDiffCostFunction<SolverError, 1, (jointConnectionCount + groundSupportCount) * 3>(
			new SolverError(referenceSystem)
		);
	}

	const Data::System & referenceSystem;
};
namespace Data {
	//----------
	template<typename T>
	TSystem<T>::Body::Body() {

	}

	//----------
	template<int jointConnectionCount, int groundSupportCount>
	void System::solve() {
		if (this->jointConnections.size() != jointConnectionCount) {
			throw("Number of joint constraints does not match templated constant jointConnectionCount");
		}
		if (this->groundSupports.size() != groundSupportCount) {
			throw("Number of ground supports does not match templated constant groundSupportCount");
		}

		double systemState[(jointConnectionCount + groundSupportCount) * 3];

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
		auto costFunction = SolverError<jointConnectionCount, groundSupportCount>::Create(*this);
		problem.AddResidualBlock(costFunction
			, NULL
			, systemState);

		ceres::Solver::Options options;
		options.linear_solver_type = ceres::DENSE_QR;
		options.max_num_iterations = 500;
		options.minimizer_progress_to_stdout = true;
		options.function_tolerance = 1e-8;
		options.line_search_direction_type = ceres::LBFGS;
		options.minimizer_type = ceres::LINE_SEARCH;
		options.num_threads = 16;
		ceres::Solver::Summary summary;
		ceres::Solve(options, &problem, &summary);
		cout << summary.FullReport() << endl;

		this->updateStateParameters(systemState);
	}
}