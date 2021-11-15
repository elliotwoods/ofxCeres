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

		bool solve();
		bool solveBasic();
		bool solveDistorted();
		bool solveGroup();

		shared_ptr<Data::CalibrationPointSet<DataPoint>> getCalibrationPoints();
		void markResidualsStale();
	protected:
		void getCalibrationData(vector<glm::vec3>& targetPoints
			, vector<glm::vec2>& panTiltSignal) const;
		void prepareDataPoint(shared_ptr<DataPoint>);

		glm::vec2 getDisparityOnDataPoint(shared_ptr<DataPoint>) const;

		DMX::MovingHead& movingHead;
		shared_ptr<Data::CalibrationPointSet<DataPoint>> calibrationPoints = make_shared<Data::CalibrationPointSet<DataPoint>>();

		struct : ofParameterGroup {
			ofParameter<SolveType> solveType{ "Solve type", SolveType::Distorted };
		} parameters;

		ofxCeres::ParameterisedSolverSettings solverSettings;
		weak_ptr<Marker> markerClosestToCursor;
		shared_ptr<Scene> scene;

		bool needsToCalculateResiduals = true;
		bool needsSolve = false;
	};
}