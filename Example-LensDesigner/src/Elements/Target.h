#pragma once

#include "Base.h"

namespace Elements {
	class Target : public Base {
	public:
		string getTypeName() const override;
		string getGlyph() const override;
	protected:
	};
}