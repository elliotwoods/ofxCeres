#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"


#define TOWER_HEIGHT 12
#define IK_SIZE (TOWER_HEIGHT * 2 + 1)

namespace SA = ofxCeres::Models::StructuralAnalysis;

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetWindowTitle("Stewart Platform");

	// Initialise the gui
	{
		this->gui.init();

		ofxAssets::Register::X().setDirectoryWatcherEnabled(true);

		this->stripPanel = ofxCvGui::Panels::Groups::makeStrip();
		{
			this->stripPanel->setCellSizes({ -1, 400 });
			this->gui.add(this->stripPanel);
		}

		// Add the panel for drawing 3D world
		{
			this->worldPanel = ofxCvGui::Panels::makeWorldManaged();
			this->worldPanel->onDrawWorld += [this](ofCamera&) {
				this->drawWorld();
				if (this->payloadParameters.draw.get()) {
					auto upperDeckTransform = this->stewartPlatform.upperDeck->getGlobalTransformMatrix();
					ofPushMatrix();
					{
						ofMultMatrix(upperDeckTransform);
						ofTranslate(0, 0, this->payloadParameters.offset.get());
						ofPushStyle();
						{
							ofNoFill();
							ofDrawCircle(0, 0, this->payloadParameters.diameter.get() / 2.0f);
						}
						ofPopStyle();
					}
					ofPopMatrix();
				}
			};
			{
				auto& camera = this->worldPanel->getCamera();
				camera.setCursorDrawEnabled(true);
				camera.setPosition({ 0, 4, 6 });
				camera.lookAt({ 0, -1, 0 });
				camera.setFov(20);
			}

			this->worldPanel->onDraw += [this](ofxCvGui::DrawArguments& args) {
				SA::DrawProperties::X().drawScaleLegend(args.localBounds);
			};
			this->worldPanel->parameters.grid.dark.set(true);
			this->worldPanel->parameters.grid.roomMin.set({ -1, -1, 0 });
			this->worldPanel->parameters.grid.roomMax.set({ 1, 1, 3 });
			this->stripPanel->add(this->worldPanel);
		}

		// Add the widgets panel
		{
			this->widgetsPanel = ofxCvGui::Panels::makeWidgets();
			this->stripPanel->add(widgetsPanel);
		}

		// Populate the widgets
		this->repopulateWidgets();
	}


	// load last save
	{
		if (ofFile::doesFileExist(LAST_SAVE_PATH)) {
			std::string filePath;
			{
				auto file = ofFile(LAST_SAVE_PATH);
				filePath = (std::string) file.readToBuffer();
				file.close();
			}

			if (ofFile::doesFileExist(filePath)) {
				this->load(filePath);
			}
		}
	}

	// setup controllers
	{
		this->controllers = ofxDualSense::Controller::listControllers();
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	for (const auto& controller : this->controllers) {
		controller->update();
		if (controller->isFrameNew()) {
			auto inputState = controller->getInputState();

			auto controllerIncline = floor(inputState.gyroscope.z * 8 + 0.5f) / 8;
			auto controllerOrientation = glm::angleAxis(
				(controllerIncline + 0.125f) * (float) TWO_PI * 2.0f
				, glm::vec3(-1, 0, 0));
			
			bool newTransform = false;

			// Translate
			{
				auto analogStickLeft = ofxDualSense::applyDeadZone(inputState.analogStickLeft, 0.125f);
				if(glm::length2(analogStickLeft) > 0.0f) {
					glm::vec3 translate{
						analogStickLeft.x
						, analogStickLeft.y
						, 0
					};
					translate = controllerOrientation * translate;
					translate *= ofGetLastFrameTime() * this->movementSpeed;
					this->stewartPlatform.upperDeck->move(translate);
					this->stewartPlatform.markNewTransformMatrix();
				}
			}

			// Rotate
			{
				auto analogStickRight = ofxDualSense::applyDeadZone(inputState.analogStickRight, 0.125f);
				if (glm::length2(analogStickRight) > 0.0f) {
					auto rotate = glm::rotation(
						glm::vec3(0, 0, 1)
						, glm::normalize(glm::vec3(analogStickRight.x, analogStickRight.y, 1)));
					rotate = controllerOrientation * rotate * glm::inverse(controllerOrientation);
					rotate = glm::slerp<float>(glm::quat{ 1, 0, 0, 0 }, rotate, ofGetLastFrameTime() * this->movementSpeed * 5.0f);
					this->stewartPlatform.upperDeck->rotate(rotate);
					this->stewartPlatform.markNewTransformMatrix();
				}
			}

			// Feedback to LED's
			{
				ofxDualSense::OutputState outputState;
				outputState.lightbar = ofColor(255, 0, 0);
				outputState.lightbar.setHueAngle(controllerIncline * 360.0f + 360.0f);
				controller->setOutputState(outputState);
			}
		}
	}
	SA::DrawProperties::X().updateMaxScalar();
	this->stewartPlatform.update();

	// OSC
	{
		// Check parameters match
		if (this->oscSender) {
			if (this->oscSender->getHost() != this->oscParameters.host.get()
				|| this->oscSender->getPort() != this->oscParameters.port.get()) {
				this->oscSender.reset();
			}
		}

		// Rebuild osc sender if needed
		if (!this->oscSender) {
			this->oscSender = make_shared<ofxOscSender>();
			this->oscSender->setup(this->oscParameters.host.get(), this->oscParameters.port.get());
		}

		// Send the message
		if (this->oscParameters.enabled.get()) {
			ofxOscMessage message;
			message.setAddress("/actuators");
			for (int i = 0; i < 6; i++) {
				message.addFloatArg(this->stewartPlatform.actuators.actuators[i]->value.get());
			}
			this->oscSender->sendMessage(message);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

}

//--------------------------------------------------------------
void ofApp::drawWorld() {
	this->stewartPlatform.draw();
	this->searchPlane.draw();
}

//--------------------------------------------------------------
void ofApp::solve() {
	this->stewartPlatform.solveForces();
}

//--------------------------------------------------------------
void ofApp::repopulateWidgets() {
	auto inspector = this->widgetsPanel;
	inspector->addFps();
	inspector->addMemoryUsage();
	inspector->addButton("Load", [this] {
		this->load();
	});
	inspector->addButton("Save", [this] {
		this->save();
	});
	inspector->addLiveValue(this->lastFilePath);
	inspector->addEditableValue(this->movementSpeed);

	SA::DrawProperties::X().populateInspector(inspector);
	inspector->addParameterGroup(this->worldPanel->parameters);

	auto drawSolved = [](const ofxCvGui::DrawArguments& drawArgs, bool solved) {
		ofPushStyle();
		if (solved) {
			ofFill();
		}
		else {
			ofNoFill();
		}
		ofDrawCircle(20
			, drawArgs.localBounds.height / 2.0f
			, 7.0f);
		ofPopStyle();
	};

	inspector->addButton("Solve forces", [this]() {
		this->solve();
		})->onDraw += [this, drawSolved](const ofxCvGui::DrawArguments & drawArgs) {
			drawSolved(drawArgs, this->stewartPlatform.isForcesSolved());
		};
	inspector->addButton("Solve IK", [this]() {
		this->stewartPlatform.solveIK();
		});
	inspector->addButton("Solve FK", [this]() {
		this->stewartPlatform.solveFK();
		})->onDraw += [this, drawSolved](const ofxCvGui::DrawArguments& drawArgs) {
			drawSolved(drawArgs, this->stewartPlatform.isFKSolved());
		};


	inspector->addParameterGroup(this->stewartPlatform);

	inspector->addParameterGroup(this->searchPlane.parameters);
	inspector->addButton("Search", [this]() {
		this->searchPlane.perform(this->stewartPlatform);
		});
	inspector->addLiveValue<size_t>("Points found", [this]() {
		return this->searchPlane.getPositions().size();
		});

	inspector->addParameterGroup(this->oscParameters);
	inspector->addParameterGroup(this->payloadParameters);
}

//--------------------------------------------------------------
void ofApp::load() {
	auto result = ofSystemLoadDialog("Save configuration");
	if (result.bSuccess) {
		this->load(result.filePath);
	}
}

//--------------------------------------------------------------
void ofApp::load(const std::string& path) {
	if (ofFile::doesFileExist(path)) {
		try {
			nlohmann::json json;
			ofFile file(path, ofFile::ReadOnly);
			file >> json;
			file.close();
			this->stewartPlatform.deserialize(json);
			this->lastFilePath = path;
			this->setLastFilePath(path);
	}
		catch (const nlohmann::json::exception& e) {
			ofSystemAlertDialog("Load " + path + " failed:\n" + std::string(e.what()));
		}
	}
}
//--------------------------------------------------------------
void ofApp::save() {
	auto result = ofSystemSaveDialog("stewartPlatform.json", "Save configuration");
	if (result.bSuccess) {
		{
			nlohmann::json json;
			this->stewartPlatform.serialize(json);
			ofFile file(result.filePath, ofFile::WriteOnly);
			file << json;
			file.close();
		}

		this->setLastFilePath(result.filePath);
	}
}

//--------------------------------------------------------------
void ofApp::setLastFilePath(const std::string& path) {
	this->lastFilePath = path;
	
	{
		ofFile file(LAST_SAVE_PATH, ofFile::WriteOnly);
		file << path;
		file.close();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 'r') {
		this->stewartPlatform.resetTransform();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
