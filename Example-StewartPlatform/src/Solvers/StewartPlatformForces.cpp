#include "pch_ofApp.h"
#include "StewartPlatformForces.h"

namespace SA = ofxCeres::Models::StructuralAnalysis;

struct StewartPlatformForcesCost
{
	StewartPlatformForcesCost(Data::StewartPlatform& stewartPlatform)
		: stewartPlatform(stewartPlatform)
	{

	}

	void
		getParameters(double * parameters) const
	{
		auto globalToLocal = glm::inverse(this->stewartPlatform.upperDeck->getOrientationQuat());

		// get forces acting on upper deck
		for (int i = 0; i < 6; i++) {
			auto actuator = this->stewartPlatform.actuators.actuators[i];

			// Get direction of actuator in world coords
			auto directionLocal = actuator->getJointPosition("upper") - actuator->getJointPosition("lower");

			//Move direction into upperDeck space 
			directionLocal = globalToLocal * directionLocal;

			// Normalise direction
			directionLocal /= glm::length(directionLocal);

			// Find joint which force is applied to
			auto connectedJointAddress = this->stewartPlatform.system.findConnectedJoint({
				actuator->value.getName()
				, "upper"
				});

			
			parameters[i] = glm::dot(this->stewartPlatform.upperDeck->joints[connectedJointAddress.jointName].force, directionLocal);
		}
	}

	template<typename T>
	void
		applyParameters(typename SA::TSystem<T>::Body& upperDeck, const T* parameters) const
	{
		auto globalToLocal = glm::inverse(this->stewartPlatform.upperDeck->getOrientationQuat());

		// Update forces acting on upper deck
		for (int i = 0; i < 6; i++) {
			auto actuator = this->stewartPlatform.actuators.actuators[i];

			// Get direction of actuator in world coords
			auto direction = actuator->getJointPosition("upper") - actuator->getJointPosition("lower");

			//Move direction into upperDeck space 
			direction = globalToLocal * direction;

			// Normalise direction
			direction /= glm::length(direction);

			// Find joint which force is applied to
			auto connectedJointAddress = this->stewartPlatform.system.findConnectedJoint({
				actuator->value.getName()
				, "upper"
				});

			upperDeck.joints[connectedJointAddress.jointName].force = parameters[i] * glm::tvec3<T>(direction);
		}
	}

	template<typename T>
	bool
		operator()(const T* const parameters
		, T* residuals) const
	{
		// Create a typed version of the upper deck
		auto upperDeckTyped = SA::TSystem<T>::Body(*this->stewartPlatform.upperDeck);
		this->applyParameters(upperDeckTyped, parameters);

		// Calculate residual
		residuals[0] = upperDeckTyped.getForceError();
		residuals[1] = upperDeckTyped.getTorqueError();

		return true;
	}

	static ceres::CostFunction* 
		Create(Data::StewartPlatform & stewartPlatform)
	{
		return new ceres::AutoDiffCostFunction<StewartPlatformForcesCost, 2, 6>(
			new StewartPlatformForcesCost(stewartPlatform)
			);
	}

	Data::StewartPlatform& stewartPlatform;
};

namespace Solvers {
	//----------
	ofxCeres::SolverSettings
		StewartPlatformForces::defaultSolverSettings()
	{
		ofxCeres::SolverSettings solverSettings;
		return solverSettings;
	}

	//----------
	StewartPlatformForces::Result
		StewartPlatformForces::solve(Data::StewartPlatform& stewartPlatform
			, bool useExistingSolution
			, const ofxCeres::SolverSettings& solverSettings)
	{
		try {
			// Initialise parameters
			double parameters[6];
			StewartPlatformForcesCost(stewartPlatform).getParameters(parameters);

			// Check for NaN's
			for (int i = 0; i < 6; i++)
			{
				if (isnan(parameters[i]))
				{
					parameters[i] = 0.0;
				}
			}

			ceres::Problem problem;
			auto costFunction = StewartPlatformForcesCost::Create(stewartPlatform);
			problem.AddResidualBlock(costFunction
				, NULL
				, parameters);

			ceres::Solver::Summary summary;

			ceres::Solve(solverSettings.options
				, &problem
				, &summary);

			if (solverSettings.printReport) {
				std::cout << summary.FullReport() << "\n";
			}

			// Update joint forces in the actual body
			{
				float floatParameters[6];
				for (int i = 0; i < 6; i++)
				{
					floatParameters[i] = (float)parameters[i];
				}
				StewartPlatformForcesCost(stewartPlatform).applyParameters(*stewartPlatform.upperDeck, floatParameters);
			}

			// construct result
			{
				Solution solution;
				memcpy(solution.forces
					, parameters
					, sizeof(double) * 6);

				Result result(summary);
				return result;
			}
		}
		catch (const ofxCeres::Exception& e) {
			Result result(e);
			ofLogError("ofxCeres") << result.errorMessage;
			return result;
		}
	}
}