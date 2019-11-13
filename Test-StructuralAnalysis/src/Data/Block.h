#pragma once

#include "ofxCeres/Models/StructuralAnalysis/System.h"

namespace Data {
	struct BaseBlock : ofxCeres::Models::StructuralAnalysis::System::Body{
		BaseBlock();
	};

	struct Block : BaseBlock{
		Block();
	};

	struct TopBlock : BaseBlock {
		TopBlock();
	};

	struct Axle : ofxCeres::Models::StructuralAnalysis::System::Body {
		Axle();
	};

	const vector<float> layerWeights {
		150
		, 150

		, 120
		, 120
		, 120
		, 120

		, 100
		, 100
		, 100

		, 80
		, 80
		, 80
		, 80
	};

	const float axleGap = 0.05f;
	const float topBearingHeight = 0.25f;
	const float bottomBearingHeight = 0.05f;
}