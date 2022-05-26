#include "pch_ofApp.h"
#include "FixtureFactory.h"

OFXSINGLETON_DEFINE(DMX::FixtureFactory);

namespace DMX {
	//----------
	FixtureFactory::FixtureFactory() {
		this->loadPlugins();
	}
}
