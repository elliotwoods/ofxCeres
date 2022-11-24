#include "pch_ofApp.h"
#include "Scene.h"

#include "Elements/PointSource.h"
#include "Elements/Boundaries/Flat.h"
#include "Elements/Boundaries/Mesh.h"
#include "Elements/Target.h"

#include "Models/OpticalSystem.h"
#include "Solvers/OpticalSystemSolver.h"

//----------
Scene::Scene() {
	this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
		this->inspect(args);
	};
	ofxCvGui::InspectController::X().onClear += [this](ofxCvGui::InspectArguments& args) {
		auto inspector = args.inspector;
		inspector->addButton("Cast", [this]() {
			this->cast();
			}, ' ');
		inspector->addToggle(this->parameters.continuousSolve);
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
			this->drawPreviewRays();
		};
		this->panel->parameters.grid.roomMin = glm::vec3(-10, -10, 0);
		this->panel->parameters.grid.roomMax = glm::vec3(10, 10, 1);
		{
			auto& camera = this->panel->getCamera();
			camera.setPosition(0, 0, 1);
			camera.lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			camera.setNearClip(0.001f);
			camera.setFarClip(100.0f);
		}
	}


	{
		this->opticalElements.emplace_back(new Elements::PointSource());
		auto element = this->getOpticalElementByType<Elements::PointSource>();
		element->setPosition({ 0, -0.1 });
		element->parameters.angle.set(100);
		element->parameters.spread.set(19.9);
	}

	{
		this->opticalElements.emplace_back(new Elements::Boundaries::Flat());
		auto boundary = this->getOpticalElementByType<Elements::Boundaries::Flat>();
		boundary->setPosition({ 0, 0 });
		boundary->setEntranceIOR(1.0);
		boundary->setExitIOR(1.5);
	}

	{
		this->opticalElements.emplace_back(new Elements::Boundaries::Mesh());
		auto boundary = this->getOpticalElementByType<Elements::Boundaries::Mesh>();
		boundary->setPosition({ -0.02, 0.0001f });
		boundary->initDefaultLine();
		boundary->setEntranceIOR(1.5);
		boundary->setExitIOR(1.0);
	}

	{
		this->opticalElements.emplace_back(new Elements::Target());
		auto element = this->getOpticalElementByType<Elements::Target>();
		element->setPosition({ -1.5, 5.0f });
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

	if (this->parameters.continuousSolve) {
		this->solve();
	}
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
	
	inspector->addParameterGroup(this->parameters);

	inspector->addSpacer();

	inspector->addSubMenu(this->panel->parameters);

	inspector->addSpacer();
	{
		inspector->addSubMenu(this->solverSettings);
		inspector->addButton("Solve", [this]() {
			try {
				this->solve();
			}
			catch (ofxCeres::Exception& exception) {
				ofSystemAlertDialog(exception.what());
			}
			}, OF_KEY_RETURN)->setHeight(100.0f);
	}
}

//----------
void
Scene::cast()
{
	auto pointSource = this->getOpticalElementByType<Elements::PointSource>();
	auto opticalSystem = this->getOpticalSystem();

	// Emit the ray chains
	vector<Models::RayChain> rayChains;
	{
		auto emittedRays = pointSource->emitRays<float>(this->parameters.resolution.get());

		for (const auto& emittedRay : emittedRays) {
			Models::RayChain newRayChain;
			newRayChain.push_back(emittedRay);
			rayChains.push_back(newRayChain);
		}
	}

	// Interact the rays
	for (auto& rayChain : rayChains) {
		opticalSystem.interact(rayChain);
	}

	this->preview.rayChains = rayChains;
}

//----------
void
Scene::drawPreviewRays()
{
	ofPushStyle();
	ofPushMatrix();
	{
		ofEnableBlendMode(ofBlendMode::OF_BLENDMODE_ADD);
		ofSetColor(this->parameters.preview.brightness.get() * 255.0f);
		glDepthMask(false);

		auto previewLength = this->parameters.preview.finalLength.get();

		ofTranslate(0, 0, 0.001f);
		for (const auto & rayChain : this->preview.rayChains) {
			for (int i = 0; i < rayChain.size() - 1; i++) {
				auto& ray = rayChain[i];
				auto& nextRay = rayChain[i + 1];
				ofDrawLine(ray.s, nextRay.s);
			}

			{
				auto& lastRay = rayChain.back();
				ofDrawLine(lastRay.s, lastRay.s + lastRay.t * previewLength);
			}
		}

		glDepthMask(true);
	}
	ofPopMatrix();
	ofPopStyle();
}

//----------
Models::OpticalSystem
Scene::getOpticalSystem() const
{
	Models::OpticalSystem opticalSystem;
	{
		for (auto element : this->opticalElements) {
			auto boundaryElement = dynamic_pointer_cast<Elements::Boundaries::Base>(element);
			if (boundaryElement) {
				auto model = boundaryElement->getOpticalModelUntyped();
				opticalSystem.opticalElements.push_back(model);
			}
		}
	}
	return opticalSystem;
}

//----------
void
Scene::solve()
{
	auto sourceElement = this->getOpticalElementByType<Elements::PointSource>();
	auto targetElement = this->getOpticalElementByType<Elements::Target>();

	auto rays = sourceElement->emitRays<float>(this->parameters.resolution.get());

	// perturb the rays
	for (auto& ray : rays) {
		ray.t.x += ofRandomf() * 0.00001f;
		ray.t.y += ofRandomf() * 0.00001f;
		ray.t = glm::normalize(ray.t);
	}

	// Create the optical system
	auto opticalSystem = this->getOpticalSystem();
	
	// Solve the system
	auto result = Solvers::OpticalSystemSolver::solve(opticalSystem
		, rays
		, targetElement->getPosition()
		, this->solverSettings.getSolverSettings());

	// Pull the models back into the boundary elements
	{
		size_t i = 0; 
		for (auto element : this->opticalElements) {
			auto boundaryElement = dynamic_pointer_cast<Elements::Boundaries::Base>(element);
			if (boundaryElement) {
				boundaryElement->setOpticalModelUntyped(result.solution.opticalSystem.opticalElements[i]);
				i++;
			}
		}
	}

	this->cast();
}