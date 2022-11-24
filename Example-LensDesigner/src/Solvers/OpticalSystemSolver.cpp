#include "pch_ofApp.h"
#include "OpticalSystemSolver.h"

struct RayTargetCost
{
	//----------
	RayTargetCost(const Models::Ray& ray
		, const glm::vec2& target
		, const Models::OpticalSystem_<double>& opticalSystem)
		: ray(ray)
		, target(target)
		, opticalSystem(opticalSystem)
	{

	}

	//----------
	template<typename T>
	bool
		operator()(T const* const* parameters
			, T* residuals)
	{
		// Initialise the optical system
		auto opticalSystem = this->opticalSystem.castTo<T>();
		opticalSystem.setParameters(parameters);

		// Initialise the ray chain
		Models::RayChain_<T> rayChain;
		rayChain.push_back(this->ray.castTo<T>());

		// Process the optics
		if (!opticalSystem.interact(rayChain)) {
			cout << "Ray missed optics" << endl;
			return false;
		}

		// Find distance to target
		auto distance = rayChain.back().distanceTo((glm::tvec2<T>) target);

		if (isnan(distance)) {
			cout << "Ray is NaN" << endl;
			return false;
		}
		else {
			residuals[0] = distance;
			return true;
		}
	}

	//----------
	static ceres::CostFunction*
		Create(const Models::Ray& ray
			, const glm::vec2& target
			, const Models::OpticalSystem_<double>& opticalSystem)
	{
		auto costFunction = new ceres::DynamicAutoDiffCostFunction<RayTargetCost, 2>(
			new RayTargetCost(ray, target, opticalSystem)
			);
		{
			for (auto& element : opticalSystem.opticalElements) {
				if (element->getParameterCount() > 0) {
					costFunction->AddParameterBlock((int)element->getParameterCount());
				}
			}
			costFunction->SetNumResiduals(1);
		}
		return costFunction;
	}

	Models::Ray ray;
	glm::vec2 target;
	Models::OpticalSystem_<double> opticalSystem;
};


struct TestCost
{
	//----------
	TestCost()
	{

	}

	//----------
	template<typename T>
	bool
		operator()(T const* parameters
			, T* residuals) const
	{
		residuals[0] = parameters[0];
		return true;
	}

	//----------
	static ceres::CostFunction*
		Create()
	{
		auto costFunction = new ceres::AutoDiffCostFunction<TestCost, 1, 4>(
			new TestCost()
			);
		return costFunction;
	}
};

namespace Solvers {
	//----------
	ofxCeres::SolverSettings
		OpticalSystemSolver::getDefaultSolverSettings()
	{
		auto solverSettings = ofxCeres::SolverSettings();
		{
			solverSettings.options.num_threads = std::thread::hardware_concurrency();
		}
		return solverSettings;
	}

	//----------
	OpticalSystemSolver::Result
		OpticalSystemSolver::solve(const Models::OpticalSystem& initialCondition
			, const vector<Models::Ray>& rays
			, const glm::vec2& target
			, const ofxCeres::SolverSettings& solverSettings)
	{
		ceres::Problem problem;

		// Copy optical system
		auto opticalSystem = initialCondition.castTo<double>();

		// Create  parameters
		vector<double*> allParameters;
		vector<std::function<void()>> delayedApplyConfig;
		{
			for (auto opticalElement : opticalSystem.opticalElements) {
				if (opticalElement->getParameterCount() > 0) {
					auto parameterCount = opticalElement->getParameterCount();
					auto parameters = new double[parameterCount];
					opticalElement->getParameters(parameters);
					allParameters.push_back(parameters);
					if (opticalElement->configureParameters) {
						delayedApplyConfig.emplace_back([opticalElement, parameters, &problem]() {
							opticalElement->configureParameters(problem, parameters);
							});
					}
				}
			}
		}

		// Add the residual blocks
		for (auto ray : rays) {
			problem.AddResidualBlock(RayTargetCost::Create(ray, target, opticalSystem)
				, NULL
				, allParameters);
		}

		// Apply upper and lower bounds
		for (auto& action : delayedApplyConfig) {
			action();
		}

		if (solverSettings.printReport) {
			cout << "Solve OpticalSystemSolver" << endl;
		}
		ceres::Solver::Summary summary;
		ceres::Solve(solverSettings.options
			, &problem
			, &summary);

		if (solverSettings.printReport) {
			cout << summary.FullReport() << endl;
		}

		// Bring parameters into models
		opticalSystem.setParameters(allParameters.data());

		// Destroy parameters
		for (auto parameters : allParameters) {
			delete[] parameters;
		}

		{
			Result result(summary);
			result.solution.opticalSystem = opticalSystem.castTo<float>();
			return result;
		}
		return ofxCeres::Exception("Not implemented");
	}
}