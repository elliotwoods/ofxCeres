#pragma once

#include "Data/Serializable.h"
#include "Data/CalibrationPointSet.h"
#include "ofxCvGui.h"

#include "Model.h"
#include "DataPoint.h"

namespace DMX {
	class MovingHead;
}

namespace Calibration {
	class Solver : public ofxCvGui::IInspectable, public Data::Serializable {
	public:
		MAKE_ENUM(SolveType
			, (Basic, Distorted, Group)
			, ("Basic", "Distorted", "Group"));

		Solver(DMX::MovingHead &);
		string getTypeName() const override;

		void drawWorld();

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);

		void solve();

		shared_ptr<Data::CalibrationPointSet<DataPoint>> getCalibrationPoints();
	protected:
		DMX::MovingHead& movingHead;
		shared_ptr<Data::CalibrationPointSet<DataPoint>> calibrationPoints = make_shared<Data::CalibrationPointSet<DataPoint>>();

		struct : ofParameterGroup {
			ofParameter<SolveType> solveType{ "Solve type", SolveType::Basic };
		} parameters;

		ofxCeres::ParameterisedSolverSettings solverSettings;
	};
}