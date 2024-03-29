#pragma once

#include "AbstractCalibrationPoint.h"

namespace Data {
	class AbstractCalibrationPointSet : public Serializable, public ofxCvGui::IInspectable {
	public:
		AbstractCalibrationPointSet();
		virtual ~AbstractCalibrationPointSet();

		void add(shared_ptr<AbstractCalibrationPoint>);
		void remove(shared_ptr<AbstractCalibrationPoint>);
		void clear();
		void clearUnselected();

		void select(shared_ptr<AbstractCalibrationPoint>);
		void selectAll();
		void selectNone();

		shared_ptr<ofxCvGui::Panels::Widgets> getListPanel();

		virtual void populateInspector(ofxCvGui::InspectArguments&);
		void serialize(nlohmann::json &);
		void deserialize(const nlohmann::json &);

		vector<shared_ptr<AbstractCalibrationPoint>> getSelectionUntyped() const;
		vector<shared_ptr<AbstractCalibrationPoint>> getAllCapturesUntyped() const;

		virtual shared_ptr<AbstractCalibrationPoint> makeEmpty() const = 0;

		ofxLiquidEvent<void> onChange;
		ofxLiquidEvent<void> onSelectionChanged;

		size_t size() const;
	protected:
		virtual bool getIsMultipleSelectionAllowed() = 0;
		vector<shared_ptr<AbstractCalibrationPoint>> captures;

		shared_ptr<ofxCvGui::Panels::Widgets> listPanel;

		bool viewDirty = true;
		bool isDeleting = false;
	};
}