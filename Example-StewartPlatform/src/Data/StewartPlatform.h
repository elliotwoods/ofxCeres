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
				"Joint spacing [m]"
				, 0.1
				, 0
				, 1
			};

			ofParameter<float> rotationOffset{
				"Rotation Offset [cycles]"
				, 1.0f / 3.0f
				, 0
				, 1
			};

			ofParameter<float> rotationX{
				"Rotation X [deg]"
				, 0
				, -180
				, 180
			};

			ofxLiquidEvent<void> onChange;

			Deck();
			void update();
			void markDirty();
			void serialize(nlohmann::json&);
			void deserialize(const nlohmann::json&);
		protected:
			void rebuild();
			ofEventListener diameterChangeListener;
			ofEventListener jointSpacingChangeListener;
			ofEventListener rotationOffsetChangeListener;
			ofEventListener rotationXChangeListener;

			bool isDirty = true;
			float jointAngleOffset = 0; // angle between joint and coincident point
			ofPolyline line;
		};

		struct Actuators : ofParameterGroup {
			ofParameter<float> minimumLength{
				"Minimum length [m]"
				, 0.787
				, 0.01
				, 3
			};

			ofParameter<float> extension{
				"Extension [m]"
				, 0.4
				, 0.01
				, 3
			};

			struct Actuator : SA::System::Body
			{
				Actuator();

				ofParameter<float> value;
				int framesSinceChange = 10;
				ofEventListener valueChangeListener;
			};
			std::shared_ptr<Actuator> actuators[6];

			ofxLiquidEvent<void> onValueChange;

			Actuators();

			ofEventListener minChangeListener;
			ofEventListener extensionChangeListener;
		} actuators;

		struct : ofParameterGroup
		{
			ofParameter<float> offsetY{ "Offset Y [m]", 0.0f, -1.0f, 1.0f };
			ofParameter<float> offsetZ{ "Offset Z [m]", 0.0f, -1.0f, 1.0f };
			ofParameter<float> mass{ "Mass [kg]", 150, 0, 300 };

			bool isDirty = true;
			PARAM_DECLARE("Weight", offsetY, offsetZ, mass);

			ofEventListener offsetYListener, offsetZListener, massListener;
		} weight;

		StewartPlatform();
		void update();
		void customDraw() override;
		void serialize(nlohmann::json&);
		void deserialize(const nlohmann::json&);
		
		void resetTransform();

		void markNeedsRebuild();
		void markNeedsFKSolve();
		void markNeedsForceSolve();
		void markNewTransformMatrix();
		void markNewTransformParameters();

		void solveForces();
		void solveIK();
		void solveFK();

		bool isForcesSolved() const;
		bool isFKSolved() const;

		bool isValidTransform(const glm::vec3& translation, const glm::vec3& rotation);

		SA::System system;

		std::shared_ptr<Deck> upperDeck;
		std::shared_ptr<Deck> lowerDeck;

		struct : ofParameterGroup {
			ofParameter<bool> reset{ "Reset [r]", false };

			struct : ofParameterGroup {
				ofParameter<float> x{ "X", 0, -1, 1 };
				ofParameter<float> y{ "Y", 1, -1, 1.5 };
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

			PARAM_DECLARE("Transform", reset, translate, rotate);
		} transform;

		struct : ofParameterGroup {
			ofParameter<bool> printOutput{ "Print output", false };
			ofParameter<int> maxIterations{ "Max iterations", 500 };
			ofParameter<bool> forcesWhenDirty{ "Forces when dirty", true };
			ofParameter<bool> IKWhenRebuild{ "IK when rebuild", true };
			ofParameter<bool> FKWhenActuatorChange{ "FK when actuator change", true };
			PARAM_DECLARE("Solve", printOutput, maxIterations, forcesWhenDirty, IKWhenRebuild, FKWhenActuatorChange);
		} solveOptions;

		struct : ofParameterGroup {
			ofParameter<bool> cycleExtremes{ "Cycle extremes", false };
			ofParameter<int> extremeIndex{ "Extreme Index", 0 };
			PARAM_DECLARE("Test", cycleExtremes, extremeIndex);
		} test;

		struct : ofParameterGroup {
			ofParameter<bool> enabled{ "Enabled", false };
			ofParameter<float> force{ "Force [N]", 1000 };
			ofParameter<glm::vec3> anchorPoint{ "Anchor", {0, 1, 0} };
			ofParameter<glm::vec3> upperDeckAttachment{ "Upper deck attachment", {0, 0, 0} };
			ofEventListener enabledChangeListener, forceChangedListener, anchorPointChangeListener, upperDeckAttachmentListener;
			PARAM_DECLARE("Counter weight", enabled, force, anchorPoint, upperDeckAttachment);
			bool isDirty = true;
		} counterWeight;

	protected:
		void transformFromParameters();
		void transformToParameters();
		void rebuild(bool allowIKSolve);
		void rebuildWeight();
		void rebuildCounterweight();
		void checkActuatorLimits();

		ofxCeres::SolverSettings getDefaultSolverSettings() const;

		bool needsRebuild = true;
		bool needsFKSolve = false;
		bool needsForceSolve = false;
		bool needsTransformFromParameters = true;
		bool needsTransformToParameters = false;

		bool forcesSolved = false;
		bool fkSolved = false;

		struct {
			float jointAngleOffsetUpper; /// offset from joints coinciding in radians
			float jointAngleOffsetLower; /// offset from joints coinciding in radians
		} cached;
	};
}