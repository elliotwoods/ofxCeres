#include "pch_ofApp.h"
#include "StewartPlatformFK.h"

namespace SA = ofxCeres::Models::StructuralAnalysis;

struct StewartPlatformFKCost
{
	StewartPlatformFKCost(Data::StewartPlatform& stewartPlatform)
		: stewartPlatform(stewartPlatform)
	{

	}

	template<typename T>
	bool
		operator()(const T* const parameters
			, T* residuals) const
	{
		const auto vectors = (glm::tvec3<T> *) parameters;
		const auto& translation = vectors[0];
		auto rotationVector = vectors[1];

		rotationVector.x = DEG_TO_RAD * rotationVector.x;
		rotationVector.y = DEG_TO_RAD * rotationVector.y;
		rotationVector.z = DEG_TO_RAD * rotationVector.z;

		auto transform = ofxCeres::VectorMath::createTransform(translation, rotationVector);

		for (int i = 0; i < 6; i++)
		{
			const auto upperJoint_ = ofxCeres::VectorMath::applyTransform(transform, (glm::tvec3<T>) this->upperDeckJointsLocal[i]);
			const auto lowerJoint = (glm::tvec3<T>) this->lowerDeckJoints[i];
			const auto actuatorLength_ = glm::distance(upperJoint_, lowerJoint);
			const auto delta = actuatorLength_ - this->actuatorLengths[i];
			residuals[i] = delta * delta;
		}

		return true;
	}

	static ceres::CostFunction*
		Create(Data::StewartPlatform& stewartPlatform)
	{
		auto newCostFunction = new StewartPlatformFKCost(stewartPlatform);

		// for each actuator, cache its end joint positions local to the upper and lower deck bodies
		for (int i = 0; i < 6; i++)
		{
			auto actuator = stewartPlatform.actuators.actuators[i];
			{
				auto connectedJointAddress = stewartPlatform.system.findConnectedJoint({
				actuator->value.getName()
				, "lower"
					});
				newCostFunction->lowerDeckJoints[i] = stewartPlatform.system.bodies[connectedJointAddress.bodyName]->getJointPosition(connectedJointAddress.jointName);
			}

			{
				auto connectedJointAddress = stewartPlatform.system.findConnectedJoint({
				actuator->value.getName()
				, "upper"
					});
				newCostFunction->upperDeckJointsLocal[i] = stewartPlatform.system.bodies[connectedJointAddress.bodyName]->joints.at(connectedJointAddress.jointName).position;
			}

			newCostFunction->actuatorLengths[i] = (double) actuator->value.get();
		}
		
		return new ceres::AutoDiffCostFunction<StewartPlatformFKCost, 6, 6>(
			newCostFunction
			);
	}

	double actuatorLengths[6];
	glm::vec3 lowerDeckJoints[6];
	glm::vec3 upperDeckJointsLocal[6];

	Data::StewartPlatform& stewartPlatform;
};

namespace Solvers
{
	//----------
	ofxCeres::SolverSettings
		StewartPlatformFK::defaultSolverSettings()
	{
		ofxCeres::SolverSettings solverSettings;
		return solverSettings;
	}

	//----------
	StewartPlatformFK::Result 
		StewartPlatformFK::solve(Data::StewartPlatform& stewartPlatform
			, bool useExistingSolution
			, const ofxCeres::SolverSettings& solverSettings)
	{
		try {
			// Pull default parametrs
			double parameters[6];
			{
				parameters[0] = (double)stewartPlatform.transform.translate.x.get();
				parameters[1] = (double)stewartPlatform.transform.translate.y.get();
				parameters[2] = (double)stewartPlatform.transform.translate.z.get();
				parameters[3] = (double)stewartPlatform.transform.rotate.x.get();
				parameters[4] = (double)stewartPlatform.transform.rotate.y.get();
				parameters[5] = (double)stewartPlatform.transform.rotate.z.get();
			}

			// Check for NaN's
			for (int i = 0; i < 6; i++)
			{
				if (isnan(parameters[i]))
				{
					parameters[i] = 0.0;
				}
			}

			// Perform the solve
			ceres::Problem problem;
			auto costFunction = StewartPlatformFKCost::Create(stewartPlatform);
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

			// Push values back into stewartPlatform
			stewartPlatform.transform.translate.x.set(parameters[0]);
			stewartPlatform.transform.translate.y.set(parameters[1]);
			stewartPlatform.transform.translate.z.set(parameters[2]);
			stewartPlatform.transform.rotate.x.set(parameters[3]);
			stewartPlatform.transform.rotate.y.set(parameters[4]);
			stewartPlatform.transform.rotate.z.set(parameters[5]);

			// Construct and return result
			{
				Solution solution;
				auto vectors = (glm::tvec3<double>*)parameters;
				solution.translation = (glm::vec3)vectors[0];
				solution.rotationVector = (glm::vec3)vectors[1];

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