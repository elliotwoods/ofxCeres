#include "ofApp.h"
#include "ofxCeres.h"

#define COUNT 100
#define NOISE 0.01

//--------------------------------------------------------------
void ofApp::setup(){
	this->gui.setup();
	this->gui.add(new ofxFloatSlider(this->parameters.noise));
	this->gui.add(new ofxToggle(this->parameters.randomizeTransform));
	this->gui.add(new ofxToggle(this->parameters.solve));

	this->randomizeTransform();
	this->solve();

	this->camera.setDistance(3.0f);
	this->camera.setNearClip(0.001f);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (this->parameters.randomizeTransform) {
		this->randomizeTransform();
		this->parameters.randomizeTransform = false;
	}

	if (this->parameters.solve) {
		this->solve();
		this->parameters.solve = false;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(40, 10);

	gui.draw();

	this->camera.begin();
	{
		ofEnableDepthTest();

		ofDrawGrid(10.0f);

		ofPushStyle();
		{
			//draw untransformed points
			ofSetColor(200, 100, 100);
			for (const auto & point : this->untransformedPoints) {
				ofDrawSphere(point, 0.01);
			}

			//draw transformed points
			ofSetColor(100, 200, 100);
			for (const auto & point : this->transformedPoints) {
				ofDrawSphere(point, 0.01);
			}

			//draw untransformed point * predicted transform
			ofSetColor(100, 100, 200);
			ofNoFill();
			ofPushMatrix();
			{
				ofMultMatrix(this->solvedTransform);
				for (const auto & point : this->untransformedPoints) {
					ofDrawCircle(point, 0.02);
				}
			}
			ofPopMatrix();
		}
		ofPopStyle();

		ofDisableDepthTest();
	}
	this->camera.end();
}

//--------------------------------------------------------------
void ofApp::randomizeTransform() {
	this->untransformedPoints.clear();
	this->transformedPoints.clear();

	// Create a random transform
	auto translation = glm::vec3(ofRandomf() + 2.0f, ofRandomf(), ofRandomf());
	auto rotationVector = glm::vec3(ofRandomf(), ofRandomf(), ofRandomf());
	auto transform = ofxCeres::VectorMath::createTransform(translation, rotationVector);

	// Synthesise some data

	for (int i = 0; i < COUNT; i++) {
		auto untransformedPoint = glm::vec3(ofRandomf(), ofRandomf(), ofRandomf());
		auto transformedPoint = transform * untransformedPoint;

		transformedPoint += glm::vec3(ofRandomf(), ofRandomf(), ofRandomf()) * this->parameters.noise.get();

		untransformedPoints.push_back(untransformedPoint);
		transformedPoints.push_back(transformedPoint);
	}
}

//--------------------------------------------------------------
void ofApp::solve() {
	auto result = ofxCeres::Models::RigidBodyTransform::solve(untransformedPoints, transformedPoints);
	this->solvedTransform = result.solution.transform;
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
