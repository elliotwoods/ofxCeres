#include "pch_ofApp.h"
#include "Solver.h"
#include "DMX/MovingHead.h"

namespace Calibration {
	//----------
	Solver::Solver(DMX::MovingHead& model)
		: movingHead(movingHead)
	{
		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;
	}

	//----------
	string
	Solver::getTypeName() const
	{
		return "Calibration::Solver";
	}

	//----------
	void
	Solver::drawWorld()
	{

	}

	//---------
	void
	Solver::serialize(nlohmann::json& json)
	{
		Data::serialize(json, this->solverSettings);

		{
			ofParameter<string> solveTypeName{ "Solve type", this->parameters.solveType.get().toString() };
			json["parameters"] << solveTypeName;
		}

		this->calibrationPoints->serialize(json);
	}

	//---------
	void
	Solver::deserialize(const nlohmann::json& json)
	{
		Data::deserialize(json, this->solverSettings);

		if (json.contains("parameters")) {
			ofParameter<string> solveTypeName{ "Solve type", "" };
			json["parameters"] >> solveTypeName;

			// set the parameter
			{
				auto solveType = this->parameters.solveType.get();
				solveType.fromString(solveTypeName);
				this->parameters.solveType.set(solveType);
			}
		}

		this->calibrationPoints->deserialize(json);
	}

	//---------
	void
	Solver::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;

		inspector->addSubMenu("Calibration points", this->calibrationPoints);

		{
			auto widget = inspector->addMultipleChoice(this->parameters.solveType.getName());
			widget->entangleManagedEnum(this->parameters.solveType);
		}

		inspector->addSubMenu(this->solverSettings);
	}

	//---------
	void
	Solver::solve()
	{

	}

	//---------
	shared_ptr<Data::CalibrationPointSet<DataPoint>>
	Solver::getCalibrationPoints()
	{
		return this->calibrationPoints;
	}
}