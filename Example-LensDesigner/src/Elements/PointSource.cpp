#include "pch_ofApp.h"
#include "PointSource.h"

namespace Elements {
	//----------
	PointSource::PointSource()
	{
		this->manageParameters(this->parameters);
	}

	//----------
	string
		PointSource::getTypeName() const
	{
		return "PointSource";
	}

	//----------
	string
		PointSource::getGlyph() const
	{
		return u8"\uf621";

		glm::vec3 i, n;
	}
}