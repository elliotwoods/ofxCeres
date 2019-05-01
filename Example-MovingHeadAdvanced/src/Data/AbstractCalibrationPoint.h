#pragma once

#include "Serializable.h"
#include "ofxCvGui.h"

namespace Data {
	class AbstractCalibrationPoint : public Serializable, public enable_shared_from_this<AbstractCalibrationPoint> {
	public:
		AbstractCalibrationPoint();
		ofxCvGui::ElementPtr getGuiElement();
		bool isSelected() const;
		void setSelected(bool);

		ofParameter<ofColor> color{ "Color", ofColor() };
		ofParameter<chrono::system_clock::time_point> timestamp{ "Timestamp", chrono::system_clock::now() };

		ofxLiquidEvent<void> onChange;
		ofxLiquidEvent<void> onDeletePressed;
		ofxLiquidEvent<bool> onSelectionChanged;
	protected:
		virtual ofxCvGui::ElementPtr getDataDisplay() = 0;
		virtual void overlayMainDisplay(ofxCvGui::ElementPtr) { }

		void rebuildDateStrings();
		void callbackSelectedChanged(bool &);

		ofParameter<bool> selected{ "Selected", true };

		string timeString;
		string secondString;
		string dateString;
	};
}