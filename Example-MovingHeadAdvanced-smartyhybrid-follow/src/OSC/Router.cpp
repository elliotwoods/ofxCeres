#include "pch_ofApp.h"
#include "Router.h"

#include <algorithm>
#include <vector>


namespace OSC {
	//----------
	void
	Router::route(const Path& path, const ofxOscMessage& message)
	{
		if (path.empty()) {
			// There isn't anything that can handle empty sub-paths
			return;
		}

		auto nextLevelName = path.front();

		// Perform static routes
		{
			for (const auto& staticRoute : this->staticRoutes) {
				if (Path::stripName(staticRoute.first) == nextLevelName) {
					staticRoute.second(message);
					return;
				}
			}
		}

		// Perform subroutes
		{
			auto subPath = path.subPath();

			for (const auto& childHandler : this->childRouters) {
				if (Path::stripName(childHandler.first) == nextLevelName) {
					childHandler.second->route(subPath, message);
					return;
				}
			}
		}

		// Perform dynamic routes
		{
			for (const auto& dynamicRoute : this->dynamicRoutes) {
				if (dynamicRoute(path, message)) {
					return;
				}
			}
		}
	}

	//----------
	void
	Router::staticRoute(const std::string& name, const std::function<void(const ofxOscMessage&)>& action)
	{
		this->staticRoutes.emplace(Path::stripName(name), action);
	}

	//----------
	void
	Router::dynamicRoute(const std::function<bool(const Path& path, const ofxOscMessage&)>& dynamicRoute)
	{
		this->dynamicRoutes.push_back(dynamicRoute);
	}

	//----------
	void
	Router::addSubRouter(const std::string& name, std::shared_ptr<Router> router)
	{
		this->childRouters.emplace(Path::stripName(name), router);
	}
}
