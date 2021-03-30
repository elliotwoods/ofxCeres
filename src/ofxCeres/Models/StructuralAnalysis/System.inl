#include "System.h"
#include "DrawProperties.h"

template<int jointConnectionCount, int groundSupportCount>
struct SolverError {
	SolverError(const ofxCeres::Models::StructuralAnalysis::System & system)
		: referenceSystem(system) {	}

	template<typename T>
	bool operator() (const T * const parameters
		, T * residuals) const {
		auto system = ofxCeres::Models::StructuralAnalysis::TSystem<T>(this->referenceSystem);
		system.updateStateParameters(parameters);
		
		auto & residual = residuals[0];
		residual = (T) 0;

		for (const auto & bodyIt : system.bodies) {
			residual += bodyIt.second->getForceError();
			residual += bodyIt.second->getTorqueError();
		}
		
		return true;
	}

	static ceres::CostFunction * Create(const ofxCeres::Models::StructuralAnalysis::System & referenceSystem) {
		return new ceres::AutoDiffCostFunction<SolverError, 1, (jointConnectionCount + groundSupportCount) * 3>(
			new SolverError(referenceSystem)
		);
	}

	const ofxCeres::Models::StructuralAnalysis::System & referenceSystem;
};

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			//----------
			template<typename T>
			std::string TSystem<T>::JointAddress::toString() const {
				return this->bodyName + "." + this->jointName;
			}

			//----------
			template<typename T>
			bool TSystem<T>::JointAddress::operator==(const JointAddress& other) const {
				return this->bodyName == other.bodyName && this->jointName == other.jointName;
			}

			//----------
			template<typename T>
			TSystem<T>::Body::Body() {

			}

			//----------
			template<typename T>
			T TSystem<T>::Body::getForceError() const {
				glm::tvec3<T> total{ 0, 0, 0 }; // Local orientation frame

				auto bodyInverseRotation = glm::inverse(this->getGlobalOrientation());

				for (const auto & loadIt : this->loads) {
					auto force = loadIt.second.force;
					if (loadIt.second.isGlobalOrientation) {
						force = bodyInverseRotation * force;
					}
					total += force;
				}

				for (const auto & jointIt : this->joints) {
					total += jointIt.second.force;
				}

				return glm::dot(total, total);
			}

			//----------
			template<typename T>
			T TSystem<T>::Body::getTorqueError() const {
				glm::tvec3<T> total{ 0, 0, 0 }; // Local orientation frame

				auto bodyInverseRotation = glm::inverse(this->getGlobalOrientation());

				for (const auto & loadIt : this->loads) {
					auto force = loadIt.second.force;
					if (loadIt.second.isGlobalOrientation) {
						force = bodyInverseRotation * force;
					}
					total += glm::cross(loadIt.second.position, force);
				}
				for (const auto & jointIt : this->joints) {
					total += glm::cross((glm::tvec3<T>) jointIt.second.position, jointIt.second.force);
				}

				return glm::dot(total, total);
			}

			//----------
			template<typename T>
			void TSystem<T>::setJointForce(const JointAddress & jointAddress
				, const glm::tvec3<T> & force
				, bool inverse) {
				auto & body = this->bodies[jointAddress.bodyName];
				auto & joint = body->joints[jointAddress.jointName];
				auto bodyInverseRotation = glm::inverse(body->getGlobalOrientation());
				joint.force = (glm::tquat<T>) bodyInverseRotation
					* force
					* (T) (inverse ? -1.0 : 1.0);
			}

			//----------
			template<int jointConnectionCount, int groundSupportCount>
			bool System::solve(const SolverSettings & solverSettings) {
				if (this->jointConnections.size() != jointConnectionCount) {
					throw(ofxCeres::Exception("Number of joint constraints does not match templated constant jointConnectionCount"));
				}
				if (this->groundSupports.size() != groundSupportCount) {
					throw(ofxCeres::Exception("Number of ground supports does not match templated constant groundSupportCount"));
				}

				this->throwIfBadJointConnection();

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

				ceres::Solver::Summary summary;
				ceres::Solve(solverSettings.options, &problem, &summary);
				
				if (solverSettings.printReport) {
					cout << summary.FullReport() << endl;
				}

				this->updateStateParameters(systemState);
				return summary.termination_type == ceres::TerminationType::CONVERGENCE;
			}
			
		}
	}
}