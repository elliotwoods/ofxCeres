#include "pch_ofApp.h"
#include "DrawProperties.h"

OFXSINGLETON_DEFINE(Data::DrawProperties);

namespace Data {
	//----------
	void DrawProperties::populateInspector(shared_ptr<ofxCvGui::Panels::Widgets> widgets) {
		widgets->addTitle("Draw properties");
		widgets->addSlider(this->sceneScale);
		widgets->addSlider(this->arrowHeadSize);
		widgets->addToggle(this->jointLabels);
		widgets->addToggle(this->loadLabels);
	}

	//----------
	float DrawProperties::getSceneScale() const {
		return exp(this->sceneScale.get());
	}

	//----------
	void DrawProperties::updateMaxScalar() {
		this->maxScalar = this->nextMaxScalar;
		this->nextMaxScalar = 1.0f;
	}

	//----------
	float DrawProperties::normalizedToScalar(float normalized) {
		return normalized * this->maxScalar;
	}

	//----------
	float DrawProperties::scalarToNormalized(float scalar) {
		if (scalar > this->nextMaxScalar) {
			this->nextMaxScalar = scalar;
		}
		return scalar / this->maxScalar;
	}

	//----------
	ofColor DrawProperties::scalarToColor(float scalar) {
		auto & scaleImage = ofxAssets::image("scale");
		if (!scaleImage.isAllocated()) {
			return ofColor(255);
		}
		else {
			auto normalized = this->scalarToNormalized(scalar);
			auto y = ofMap(normalized, 0, 1.0f, 0, scaleImage.getHeight() - 1, true);
			return scaleImage.getPixels().getColor(scaleImage.getWidth() / 2, y);
		}
	}

	//----------
	float DrawProperties::getScalarToSceneScale() {
		return this->getSceneScale() / this->maxScalar;
	}

	//----------
	void DrawProperties::drawScaleLegend(const ofRectangle & viewBounds) const {
		ofPushMatrix();
		{
			ofTranslate(viewBounds.getBottomRight() - glm::vec2(120, 220));

			auto & scaleImage = ofxAssets::image("scale");
			scaleImage.draw(0, 200, 32, -200);

			ofPushStyle();
			{
				ofNoFill();
				ofDrawRectangle(0, 200, 32, -200);
			}
			ofPopStyle();

			for (float norm = 0.0f; norm <= 1.0f; norm += 0.2f) {
				auto scalar = DrawProperties::X().normalizedToScalar(norm);
				ofDrawBitmapString(ofToString(scalar, 1) + "N", 40, ofMap(norm, 0, 1, 200, 0));
			}
		}
		ofPopMatrix();
		
	}
}