#include "pch_ofApp.h"
#include "Scene.h"

#include "Elements/PointSource.h"
#include "Elements/Boundaries/Flat.h"
#include "Elements/Boundaries/Mesh.h"

//----------
Scene::Scene() {
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
		this->inspect(args);
	};

	{
		this->panel = ofxCvGui::Panels::makeWorldManaged();
		this->panel->onUpdate += [this](ofxCvGui::UpdateArguments& args) {
			auto& camera = this->panel->getCamera();
			camera.setOrientation(glm::quat());
		};
		this->panel->onDrawWorld += [this](ofCamera&) {
			for (auto opticalElement : this->opticalElements) {
				opticalElement->drawWorldSpace();
			}
			this->drawTest();
		};
		this->panel->parameters.grid.roomMin = glm::vec3(-10, -10, 0);
		this->panel->parameters.grid.roomMax = glm::vec3(10, 10, 1);
		{
			auto& camera = this->panel->getCamera();
			camera.setPosition(0, 0, 1);
			camera.lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		}
	}


	this->opticalElements.emplace_back(new Elements::PointSource());
	this->opticalElements.emplace_back(new Elements::Boundaries::Flat());
	{
		auto boundary = this->getOpticalElementByType<Elements::Boundaries::Flat>();
		boundary->setPosition({ 0, 0.3f });
		boundary->setEntranceIOR(1.0);
		boundary->setExitIOR(1.5);

	}
	this->opticalElements.emplace_back(new Elements::Boundaries::Mesh());
	{
		auto boundary = this->getOpticalElementByType<Elements::Boundaries::Mesh>();
		boundary->initCurve(1, PI / 4);
		//boundary->initLine(1.0);
		boundary->setPosition({ 0, 0.5f });
		boundary->setEntranceIOR(1.5);
		boundary->setExitIOR(1.0);
	}

	for (auto opticalElement : this->opticalElements) {
		opticalElement->init();
	}
}

//----------
void
Scene::update()
{
//	// Expand canvas bounds to contain all elements
//	{
//		auto bounds = ofRectangle(0, 0, 0, 0);
//		for (auto opticalElement : this->opticalElements) {
//			auto elementBounds = opticalElement->getBounds();
//			if (elementBounds.getRight() > bounds.getRight() {
//				bounds.width = elementBounds.getRight() - bounds.x;
//			}
//			if (elementBounds.getBottom() > bounds.getBottom() {
//				bounds.height = elementBounds.getBottom() - bounds.y;
//			}
//
//			if (elementBounds.getLeft() < bounds.getLeft()) {
//				auto expansion = bounds.x - elementBounds.x;
//					bounds.width += expansion;
//					bounds.x -= expansion;
//			}
//
//			if (elementBounds.getTop() < bounds.getTop()) {
//				auto expansion = bounds.y - elementBounds.y;
//				bounds.height += expansion;
//				bounds.y -= expansion;
//			}
//		}
//
//	}
}

//----------
void
Scene::inspect(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;

	inspector->addTitle("Optical elements");
	for (const auto& element : this->opticalElements) {
		auto button = inspector->addSubMenu(element->getName()
			, element
			, false);

		auto glyph = element->getGlyph();

		button->onDraw += [glyph](ofxCvGui::DrawArguments& args) {
			auto bounds = args.localBounds;
			bounds.width = bounds.height;
			ofxCvGui::Utils::drawGlyph(glyph, bounds);
		};
	}

	inspector->addSpacer();

	inspector->addSubMenu(this->panel->parameters);
	inspector->addButton("Test", [this]() {
		this->test();
		});
}

//----------
void
Scene::test()
{
	auto pointSource = this->getOpticalElementByType<Elements::PointSource>();
	auto flatBoundary = this->getOpticalElementByType<Elements::Boundaries::Flat>();
	auto meshBoundary = this->getOpticalElementByType<Elements::Boundaries::Mesh>();
	auto flatBoundaryModel = flatBoundary->getModel<float>();
	auto meshBoundaryModel = meshBoundary->getModel<float>();

	// Emit the rays
	vector<shared_ptr<RayChainLink>> rayChainLinks;
	{
		auto emittedRays = pointSource->emitRays<float>(100);

		for (const auto& emittedRay : emittedRays) {
			auto rayChainLink = make_shared<RayChainLink>();
			{
				rayChainLink->ray = emittedRay;
				rayChainLink->source = pointSource;
			}
			rayChainLinks.push_back(rayChainLink);
		}
	}

	// Interact with the boundaries
	for (auto& rayChainLink : rayChainLinks) {
		auto emittedRay = flatBoundaryModel.interact(rayChainLink->ray);
		if (emittedRay) {
			rayChainLink->nextRay = make_shared<RayChainLink>();
			{
				rayChainLink->nextRay->ray = * emittedRay;
				rayChainLink->nextRay->source = flatBoundary;
			}

			auto emittedRay2 = meshBoundaryModel.interact(*emittedRay);
			if (emittedRay2) {
				rayChainLink->nextRay->nextRay = make_shared<RayChainLink>();
				{
					rayChainLink->nextRay->nextRay->ray = *emittedRay2;
					rayChainLink->nextRay->nextRay->source = meshBoundary;
				}
			}
		}
	}

	this->testResults.rayChains = rayChainLinks;
}

//----------
void
Scene::drawTest()
{
	ofPushMatrix();
	{
		ofTranslate(0, 0, 0.001f);
		for (const auto & rayChainStart : this->testResults.rayChains) {
			auto rayChain = rayChainStart;
			while (rayChain->nextRay) {
				ofDrawLine(rayChain->ray.s, rayChain->nextRay->ray.s);
				rayChain = rayChain->nextRay;
			}
			ofDrawLine(rayChain->ray.s, rayChain->ray.s + rayChain->ray.t);
		}
	}
	ofPopMatrix();
}