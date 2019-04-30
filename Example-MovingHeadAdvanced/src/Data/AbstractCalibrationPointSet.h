#pragma once

#include "AbstractCalibrationPoint.h"

namespace Data {
	class AbstractCalibrationPointSet : public Serializable {
	public:
		AbstractCalibrationPointSet();
		virtual ~AbstractCalibrationPointSet();

		void add(shared_ptr<AbstractCalibrationPoint>);
		void remove(shared_ptr<AbstractCalibrationPoint>);
		void clear();

		void select(shared_ptr<AbstractCalibrationPoint>);
		void selectAll();
		void selectNone();

		void populateWidgets(shared_ptr<ofxCvGui::Panels::Widgets>);
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

		shared_ptr<ofxCvGui::Panels::Widgets> listView;

		bool viewDirty = true;
	};
}