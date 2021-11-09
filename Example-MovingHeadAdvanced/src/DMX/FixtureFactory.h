#pragma once

#include "ofxPlugin.h"
#include "Fixture.h"

namespace DMX {
	class FixtureFactory : public ofxPlugin::FactoryRegister<Fixture>, public ofxSingleton::Singleton<FixtureFactory> {
	public:
		FixtureFactory();
	};
}