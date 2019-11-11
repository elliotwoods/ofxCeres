#pragma once

#include "System.h"

namespace Data {
	struct BaseBlock : System::Body{
		BaseBlock();
	};

	struct Block : BaseBlock{
		Block();
	};

	struct TopBlock : BaseBlock {
		TopBlock();
	};

	struct Axle : System::Body {
		Axle();
	};
}