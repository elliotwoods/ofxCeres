#pragma once
#include "ofParameter.h"
#include "ofPolyline.h"
#include "ofxCvGui/Utils/Sugar.h"
#include "ofxCeres.h"

namespace SA = ofxCeres::Models::StructuralAnalysis;

namespace Data
{
	struct StewartPlatform : ofParameterGroup, ofNode {
		struct Deck : ofParameterGroup, SA::System::Body {
			ofParameter<float> diameter{
				"Diameter [m]"
				, 1
				, 0.01
				, 5
			};

			ofParameter<float> jointSpacing{
				"Join spacing [m]"
				, 0.1
				, 0
				, 1
			};

			ofxLiquidEvent<void> onChange;

			Deck();
			void update();
			void markDirty();
		protected:
			void rebuild();
			ofEventListener diameterChangeListener;
			ofEventListener jointSpacingChangeListener;

			bool isDirty = true;
			float jointAngleOffset = 0; // angle between joint and coincident point
			ofPolyline line;
		};

		struct Actuators : ofParameterGroup {
			ofParameter<float> minimumLength{
				"Minimum length [m]"
				, 0.4
				, 0.01
				, 5
			};

			ofParameter<float> maximumLength{
				"Maximum length [m]"
				, 0.8
				, 0.01
				, 5
			};

			struct Actuator : SA::System::Body
			{
				Actuator();

				ofParameter<float> value;
			};
			std::shared_ptr<Actuator> actuators[6];

			ofxLiquidEvent<void> onValueChange;
			
			Actuators();

			ofEventListener maxChangeListener;
			ofEventListener minChangeListener;
			ofEventListener valueChangeListener[6];
		} actuators;

		struct : ofParameterGroup
		{
			ofParameter<float> offset{ "Offset [m]", 0.5f, -1.0f, 1.0f };
			ofParameter<float> mass{ "Mass [kg]", 150, 0, 300 };

			bool isDirty = true;
			PARAM_DECLARE("Weight", offset, mass);

			ofEventListener offsetListener, massListener;
		} weight;

		StewartPlatform();
		void update();
		void customDraw() override;
		
		void markNeedsRebuild();
		void markNeedsFKSolve();
		void markNeedsForceSolve();

		void solveForces();
		void solveIK();
		void solveFK();

		bool isForcesSolved() const;
		bool isFKSolved() const;

		SA::System system;

		std::shared_ptr<Deck> upperDeck;
		std::shared_ptr<Deck> lowerDeck;

		struct : ofParameterGroup {
			struct : ofParameterGroup {
				ofParameter<float> x{ "X", 0, -1, 1 };
				ofParameter<float> y{ "Y", 1, 0.5, 1.5 };
				ofParameter<float> z{ "Z", 0, -1, 1 };
				ofEventListener changeListenerX, changeListenerY, changeListenerZ;
				PARAM_DECLARE("Translate", x, y, z);
			} translate;
			
			struct : ofParameterGroup {
				ofParameter<float> x{ "X", 0, -180, 180 };
				ofParameter<float> y{ "Y", 0, -180, 180 };
				ofParameter<float> z{ "Z", 0, -180, 180 };
				ofEventListener changeListenerX, changeListenerY, changeListenerZ;
				PARAM_DECLARE("Rotate", x, y, z);
			} rotate;

			PARAM_DECLARE("Transform", translate, rotate);
		} transform;

		struct : ofParameterGroup {
			ofParameter<bool> forcesWhenDirty{ "Forces when dirty", true };
			ofParameter<bool> IKWhenRebuild{ "IK when rebuild", true };
			ofParameter<bool> FKWhenActuatorChange{ "FK when actuator change", true };
			PARAM_DECLARE("Solve", forcesWhenDirty, IKWhenRebuild, FKWhenActuatorChange);
		} solveOptions;

	protected:
		void rebuild(bool allowIKSolve);
		void rebuildWeight();

		bool needsRebuild = true;
		bool needsFKSolve = false;
		bool needsForceSolve = false;

		bool forcesSolved = false;
		bool fkSolved = false;

		struct {
			float jointAngleOffsetUpper; /// offset from joints coinciding in radians
			float jointAngleOffsetLower; /// offset from joints coinciding in radians
		} cached;
	};
}