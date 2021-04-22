#pragma once

#include "ofxCvGui.h"
#include "../Data/StewartPlatform.h"

namespace Procedure {
	class SearchPlane {
	public:
		struct : ofParameterGroup {
			ofParameter<float> initialStepSize{ "Initial step size [m]", 0.1, 0.01, 2 };
			ofParameter<int> stepDepth{ "Step depth", 16 };
			PARAM_DECLARE("SearchPlane", initialStepSize, stepDepth);
		} parameters;

		/// <summary>
		/// Draw to the 3D world
		/// </summary>
		void draw();

		void perform(Data::StewartPlatform& stewartPlatform);
		const std::vector<glm::vec3> & getPositions() const;

	protected:
		void performAtPoint(Data::StewartPlatform& stewartPlatform
			, const glm::vec3& position
			, float stepSize
			, int remainingIterations);
		std::vector<glm::vec3> validPositions;
		ofMesh preview;
	};
}