#include "pch_ofApp.h"
#include "Path.h"

namespace OSC {
	//----------
	Path::Path()
	{

	}

	//----------
	Path::Path(const string& address)
	{
		auto rawPath = ofSplitString(address, "/", true);
		for (auto level : rawPath) {
			this->push_back(Path::stripName(level));
		}
	}

	//----------
	Path
		Path::subPath() const
	{
		if (this->empty()) {
			return Path();
		}
		else {
			Path subPath;
			subPath.assign(this->begin() + 1, this->end());
			return subPath;
		}
	}

	//----------
	string
		Path::stripName(const string& nameIn)
	{
		// Lower case
		auto nameStripped = ofToLower(nameIn);

		// Remove spaces
		nameStripped.erase(std::remove(nameStripped.begin(), nameStripped.end(), ' '), nameStripped.end());
		return nameStripped;
	}
}
