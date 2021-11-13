#include "pch_ofApp.h"
#include "GroupControl.h"
#include "Scene.h"

//----------
GroupControl::GroupControl()
{
	RULR_SERIALIZE_LISTENERS;
	RULR_INSPECTOR_LISTENER;

	this->staticRoute("navigateTo", [this](const ofxOscMessage& message) {
		if (message.getNumArgs() >= 3) {
			auto position = glm::vec3(message.getArgAsFloat(0)
				, message.getArgAsFloat(1)
				, message.getArgAsFloat(2));
			this->navigateTo(position);
		}
		});

	this->dynamicRoute([](const OSC::Path& path, const ofxOscMessage& message) {
		// Pass all unhandled routes over to the moving heads
		auto& movingHeads = Scene::X()->getMovingHeads();
		for (const auto& movingHead : movingHeads) {
			movingHead.second->route(path, message);
		}
		return true;
		});
}

//----------
string
GroupControl::getTypeName() const
{
	return "GroupControl";
}

//----------
void
GroupControl::init()
{
	// Shouldn't be called on the constructor
	Scene::X()->getPanel()->onMouse += [this](ofxCvGui::MouseArguments&) {
		this->needsWorldUpdate = true;
	};
	this->initialised = true;
}

//----------
void
GroupControl::update()
{
	if (!this->initialised) {
		this->init();
	}

	if (this->needsWorldUpdate && this->parameters.trackCursor) {
		auto cursorPosition = Scene::X()->getPanel()->getCamera().getCursorWorld();
		this->navigateTo(cursorPosition);
		this->needsWorldUpdate = false;
	}

	// trim history
	while (this->history.size() > this->parameters.draw.historyTrail.length.get()) {
		this->history.pop_front();
	}
}

//----------
void
GroupControl::drawWorld()
{
	if (this->parameters.draw.historyTrail.enabled) {
		ofPushStyle();
		{
			ofEnableAlphaBlending();
			ofMesh trail;
			trail.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINE_STRIP);
			ofColor color(255);
			size_t trailIndex = 0;
			float historyLength = this->history.size();
			for (auto position : this->history) {
				trail.addVertex(position);
				color.a = ofMap(trailIndex++
					, 0, historyLength
					, 0.0f, 255.0f
					, false);
				trail.addColor(color);
			}

			glPushAttrib(GL_POLYGON_BIT);
			{
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(1.0, -2.0);
				trail.draw();
			}
			glPopAttrib();
		}
		ofPopStyle();
	}
}

//----------
void
GroupControl::serialize(nlohmann::json& json)
{
	Data::serialize(json, this->parameters);
}

//----------
void
GroupControl::deserialize(const nlohmann::json& json)
{
	Data::deserialize(json, this->parameters);
}

//----------
void
GroupControl::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;
	inspector->addParameterGroup(this->parameters);
}

//----------
void
GroupControl::navigateTo(const glm::vec3& worldPosition)
{
	auto movingHeads = Scene::X()->getMovingHeads();
	for (const auto& movingHead : movingHeads) {
		movingHead.second->navigateToWorldTarget(worldPosition);
	}
	this->history.push_back(worldPosition);
}