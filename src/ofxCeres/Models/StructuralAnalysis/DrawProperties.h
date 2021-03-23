#pragma once
#include "ofParameter.h"
#include "ofGraphics.h"

#ifdef HAS_OFXCVGUI
#include "ofxCvGui.h"
#endif

namespace ofxCeres {
	namespace Models {
		namespace StructuralAnalysis {
			class DrawProperties {
			public:
				static DrawProperties & X();
#ifdef HAS_OFXCVGUI
				void populateInspector(std::shared_ptr<ofxCvGui::Panels::Widgets>);
#endif
				float getSceneScale() const;

				void updateMaxScalar();
				float normalizedToScalar(float);
				float scalarToNormalized(float);
				float getScalarToSceneScale();

#ifdef HAS_OFXCVGUI
				ofColor scalarToColor(float);
				void drawScaleLegend(const ofRectangle & viewBounds) const;
#endif

				ofParameter<float> sceneScale{ "Scene scale x10^x", -5, -10, 2 };
				ofParameter<float> arrowHeadSize{ "Arrow head size", 0.005, 0.001, 0.1 };
				ofParameter<float> anchorSize{ "Anchor size", 0.03, 0.001, 1 };
				ofParameter<bool> jointLabels{ "Joint labels", false };
				ofParameter<bool> loadLabels{ "Load labels", false };

				float maxScalar = 1;
				float nextMaxScalar = 1;
			};
		}
	}
}