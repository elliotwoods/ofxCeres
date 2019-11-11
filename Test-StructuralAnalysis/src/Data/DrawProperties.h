#pragma once
#include "ofxSingleton.h"
#include "ofParameter.h"
#include "ofxCvGui.h"

namespace Data {
	class DrawProperties : public ofxSingleton::Singleton<DrawProperties> {
	public:
		void populateInspector(shared_ptr<ofxCvGui::Panels::Widgets>);
		float getSceneScale() const;
		
		void updateMaxScalar();
		float normalizedToScalar(float) ;
		float scalarToNormalized(float);
		ofColor scalarToColor(float);
		float getScalarToSceneScale();
		void drawScaleLegend(const ofRectangle & viewBounds) const;

		ofParameter<float> sceneScale{ "Scene scale x10^x", -2, -5, 2 };
		ofParameter<float> arrowHeadSize{ "Arrow head size", 0.01, 0.001, 1 };
		ofParameter<float> anchorSize{ "Anchor size", 0.03, 0.001, 1 };
		ofParameter<bool> jointLabels{ "Joint labels", true };
		ofParameter<bool> loadLabels{ "Load labels", true };

		float maxScalar = 1;
		float nextMaxScalar = 1;
	};
}