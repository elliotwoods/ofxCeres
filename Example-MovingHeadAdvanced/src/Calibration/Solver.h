#pragma once

#include "Data/Serializable.h"
#include "Data/CalibrationPointSet.h"
#include "ofxCvGui.h"

#include "Model.h"
#include "DataPoint.h"
#include "Markers.h"

namespace DMX {
	class MovingHead;
}

class Scene;

namespace Calibration {
	class Solver : public ofxCvGui::IInspectable, public Data::Serializable {
	public:
		MAKE_ENUM(SolveType
			, (Basic, Distorted, Group)
			, ("Basic", "Distorted", "Group"));

		Solver(DMX::MovingHead &);
		~Solver();

		string getTypeName() const override;

		void update();
		void drawWorld();
		void drawRaysAndResiduals();

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);

		void addCalibrationPoint();

		void solve();
		void solveBasic();
		void solveDistorted();
		void solveGroup();

		shared_ptr<Data::CalibrationPointSet<DataPoint>> getCalibrationPoints();
	protected:
		void getCalibrationData(vector<glm::vec3>& targetPoints
			, vector<glm::vec2>& panTiltSignal) const;
		void prepareDataPoint(shared_ptr<DataPoint>);
		float getResidualOnDataPoint(shared_ptr<DataPoint>) const;

		DMX::MovingHead& movingHead;
		shared_ptr<Data::CalibrationPointSet<DataPoint>> calibrationPoints = make_shared<Data::CalibrationPointSet<DataPoint>>();

		struct : ofParameterGroup {
			ofParameter<SolveType> solveType{ "Solve type", SolveType::Basic };
		} parameters;

		ofxCeres::ParameterisedSolverSettings solverSettings;
		weak_ptr<Marker> markerClosestToCursor;
		shared_ptr<Scene> scene;

		float maxResidual = 1;
	};
}