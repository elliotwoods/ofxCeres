#pragma once

#include "ofxCvGui.h"
#include "../Data/StewartPlatform.h"

namespace Procedure {
	class SearchPlane {
	public:
		struct : ofParameterGroup {
			ofParameter<float> z{ "Z", 0.7, 0, 2.0 };
			ofParameter<float> windowSize{ "Window Size [m]", 0.4, 0.1, 3.0 };
			ofParameter<int> resolution{ "Resolution", 16, 1, 256 };
			PARAM_DECLARE("SearchPlane", z, windowSize, resolution);
		} parameters;

		/// <summary>
		/// Draw to the 3D world
		/// </summary>
		void draw();

		void perform(Data::StewartPlatform& stewartPlatform);
		const std::vector<glm::vec3> & getPositions() const;

	protected:
		std::vector<glm::vec3> validPositions;
	};
}