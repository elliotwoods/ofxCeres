#include "ofApp.h"
#include "ofxCeres.h"

#define COUNT 100
#define NOISE 0.01

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(true);
	this->camera.setFixUpDirectionEnabled(true);

	this->gui.setup();
	this->gui.add(new ofxVec3Slider(this->parameters.translation));
	this->gui.add(new ofxVec3Slider(this->parameters.rotation));
	this->gui.add(new ofxFloatSlider(this->parameters.tiltOffset));

	this->gui.add(new ofxToggle(this->parameters.drawGrid));
	this->gui.add(new ofxToggle(this->parameters.showCursor));

	this->gui.add(new ofxToggle(this->parameters.solve));

	this->names.insert(this->names.end(), {
		"A"
		, "C"
		, "D"
		, "Y"
		, "E"
		, "F"
		});

	this->panTiltDMXValues.insert(this->panTiltDMXValues.end(),
		{
			{ {203 , 112} , {21 , 40} }
			, { {161 , 47} , {22 , 20} }
			, { {151 , 16} , {22 , 39} }
			, { {0 , 0} , {22 , 19} }
			, { {151 , 40} , {59 , 5} } // F and E are switched
			, { {151 , 16} , {3 , 5} } // F and E are switched
		}
		);

	this->targetPoints.insert(this->targetPoints.end(),
		{
			{-2.20, -2.56, 0.78}
			, {3.76, 0.00, 0.78}
			, {3.76, -2.45, 0.78}
			, {2.35, 0.00, 0.78}
			, {3.76, -2.45, 2.32}
			, {3.76, -2.45, 0}
		});

	// SketchUp to openFrameworks coordinates
	for (auto & targetPoint : targetPoints) {
		targetPoint = glm::vec3(targetPoint.x, targetPoint.z, -targetPoint.y);
	}

	//Translate DMX values into panTilt values
	// Pan is -270 to +270 degrees
	// Tilt is -135 to +135 degrees
	const auto dmxMaxValue = std::numeric_limits<uint16_t>::max();
	for (const auto & dmxValues : this->panTiltDMXValues) {
		this->panTiltAngles.emplace_back(
			ofMap(dmxValues.pan.getRatio(), 0, 1.0f, -270, +270)
			, ofMap(dmxValues.tilt.getRatio(), 0, 1.0f, -130, +130));

		ofColor previewColor(200, 100, 100);
		previewColor.setHueAngle(ofRandom(360));
		this->previewColors.push_back(previewColor);
	}

	//this->solve();
}

//--------------------------------------------------------------
void ofApp::update(){
	this->camera.setCursorDrawEnabled(this->parameters.showCursor);

	if (this->parameters.solve) {
		this->solve();
		this->parameters.solve = false;
	}

	this->solvedTransform = ofxCeres::VectorMath::createTransform(this->parameters.translation.get(), this->parameters.rotation.get());
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(40, 10);

	this->camera.begin();
	{
		ofEnableDepthTest();

		if (this->parameters.drawGrid) {
			ofPushStyle();
			{
				ofSetColor(150);

				ofPushMatrix();
				{
					ofRotateDeg(180, 0, 1, 0);
					ofRotateDeg(90, 0, 0, 1);
					ofDrawGridPlane(1.0f, 10, true);
				}
				ofPopMatrix();
			}
			ofPopStyle();
		}

		ofPushStyle();
		{
			//draw target points
			auto previewColor = this->previewColors.data();
			{
				int index = 0; 
				for (const auto & point : this->targetPoints) {
					ofSetColor(*previewColor++);
					ofDrawSphere(point, 0.03);

					ofSetColor(255);
					ofDrawBitmapString(this->names[index], point);
					index++;
				}
			}

			//draw moving head
			ofNoFill();
			vector<glm::vec3> transmissionsInObjectSpace;
			ofPushMatrix();
			{
				ofMultMatrix(this->solvedTransform);

				//draw hardware
				ofSetColor(150);
				ofDrawAxis(0.4f);
				ofSetSphereResolution(6);
				ofDrawBox(glm::vec3(0, -0.35, 0), 0.5, 0.1, 0.4);
				ofPushMatrix();
				{
					ofScale(0.6, 1, 0.6);
					ofDrawSphere(0.2f);
				}
				ofPopMatrix();

				//draw rays
				ofSetColor(255);
				ofSetLineWidth(2.0f);
				previewColor = this->previewColors.data();
				{
					int index = 0;
					for (const auto & panTiltAngle : this->panTiltAngles) {
						ofSetColor(*previewColor++);
						auto transmission = ofxCeres::VectorMath::getObjectSpaceRayForPanTilt(panTiltAngle, this->parameters.tiltOffset.get());

						ofDrawLine(glm::vec3(), transmission);
						transmissionsInObjectSpace.push_back(transmission);

						ofDrawBitmapString(this->names[index] , transmission);
						index++;
					}
				}
			}
			ofPopMatrix();

			//draw residuals
			ofPushStyle();
			{
				ofSetColor(100);
				ofSetLineWidth(1.0f);
				for (int i = 0; i < this->targetPoints.size(); i++) {
                    auto transmissionTipInWorldSpace = this->solvedTransform * glm::vec4(transmissionsInObjectSpace[i], 1.0f);
                    ofDrawLine((glm::vec3) transmissionTipInWorldSpace, this->targetPoints[i]);
				}
			}
			ofPopStyle();
		}
		ofPopStyle();

		ofDisableDepthTest();
	}
	this->camera.end();

	gui.draw();
}

//--------------------------------------------------------------
void ofApp::solve() {
	auto result = ofxCeres::Models::MovingHead::solve(this->targetPoints
		, this->panTiltAngles
		, ofxCeres::Models::MovingHead::Solution {
			this->parameters.translation.get()
			, this->parameters.rotation.get()
			, this->parameters.tiltOffset.get()
		});
	this->parameters.translation = result.solution.translation;
	this->parameters.rotation = result.solution.rotationVector;
	this->parameters.tiltOffset = result.solution.tiltOffset;

	this->camera.lookAt(this->parameters.translation.get());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
