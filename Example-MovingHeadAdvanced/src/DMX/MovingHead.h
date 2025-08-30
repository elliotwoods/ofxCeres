#pragma once

#include "Fixture.h"

#include "Calibration/Model.h"
#include "Calibration/Solver.h"

namespace DMX {
	class MovingHead : public Fixture {
	public:
		struct Configuration {
			float minimumPan = -270;
			float maximumPan = 270;
			float minimumTilt = -125;
			float maximumTilt = 125;
		};

		MovingHead(const Configuration &);

		void update() override;
		void drawWorld() override;

		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		void populateInspector(ofxCvGui::InspectArguments&);

		glm::vec2 getCurrentPanTilt() const;
		void navigateToWorldTarget(const glm::vec3&);

		shared_ptr<Calibration::Model> getModel();
		shared_ptr<Calibration::Solver> getSolver();

		void flip();
		void toggleSolo();

		bool getSolo() const;
		void setSolo(bool);

		struct : ofParameterGroup {
			ofParameter<bool> shutterOpen{ "Shutter open", false }; // true = closed
			ofParameter<float> pan{ "Pan", 0, -180, 180 };
			ofParameter<float> tilt{ "Tilt", 0, -90, 90 };
			ofParameter<float> dimmer{ "Dimmer", 0, 0, 1};
			ofParameter<float> focus{ "Focus", 0, 0, 1};
			ofParameter<ofRectangle> boundsLimit{ "Bounds limit", ofRectangle() };
			PARAM_DECLARE("MovingHead", shutterOpen, pan, tilt, dimmer, focus, boundsLimit);
		} parameters;

	protected:
		shared_ptr<Calibration::Model> model = make_shared<Calibration::Model>(*this);
		shared_ptr<Calibration::Solver> solver = make_shared<Calibration::Solver>(*this);

		friend Calibration::Model;
		friend Calibration::Solver;
	};
}