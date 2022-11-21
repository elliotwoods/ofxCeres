#pragma once

#include "Path.h"
#include "ofxOscMessage.h"

namespace OSC {

	class Router {
	public:
		void route(const Path& path, const ofxOscMessage&);
	protected:
		void staticRoute(const std::string& name, const std::function<void(const ofxOscMessage&)>&);
		void dynamicRoute(const std::function<bool(const Path& path, const ofxOscMessage&)>&);
		void addSubRouter(const std::string& name, std::shared_ptr<Router>);
	private:
		std::map<std::string, std::shared_ptr<Router>> childRouters;
		std::map<std::string, std::function<void(const ofxOscMessage&)>> staticRoutes;
		std::vector<std::function<bool(const Path& path, const ofxOscMessage&)>> dynamicRoutes;
	};
}
