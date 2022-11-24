#include "pch_ofApp.h"
#include "Target.h"

namespace Elements {
	//----------
	string
		Target::getTypeName() const
	{
		return "Target";
	}

	//----------
	string
		Target::getGlyph() const
	{
		return u8"\uf140";
	}
}