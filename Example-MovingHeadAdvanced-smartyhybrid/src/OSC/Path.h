#pragma once

#include "ofxOscMessage.h"
#include <list>
#include <map>

namespace OSC {
	class Path : public std::vector<std::string>
	{
	public:
		Path();
		Path(const std::string& address);
		Path subPath() const;
		static std::string stripName(const std::string&);
	};
}
