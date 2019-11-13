#include "pch_ofxCeres.h"
#include "System.h"

namespace ofxCeres {
	namespace Models {
		namespace InverseKinematics {
			//-----------
			SolverSettings System::getDefaultSolverSettings() {
				auto solverSettings = ofxCeres::SolverSettings();
				auto & options = solverSettings.options;
				{
					options.linear_solver_type = ceres::DENSE_QR;
					options.num_threads = 16;
					options.function_tolerance = 1e-10;
					options.line_search_direction_type = ceres::LBFGS;
					options.minimizer_type = ceres::LINE_SEARCH;
				}
				return solverSettings;
			}

			//-----------
			std::vector<PositionAndRotation> System::getCurrentPositionsAndRotations() const {
				std::vector<PositionAndRotation> PandRs(this->bodyLengths.size() + 1);
				this->getPandRsForState(this->currentRotationState.data(), PandRs.data());
				return PandRs;
			}

			//-----------
			void System::draw() {
				auto PandRs = this->getCurrentPositionsAndRotations();
				ofPolyline line;
				for(const auto & PandR : PandRs) {
					line.addVertex(PandR.position.x, PandR.position.y);
				}
				line.draw();
			}
		}
	}
}