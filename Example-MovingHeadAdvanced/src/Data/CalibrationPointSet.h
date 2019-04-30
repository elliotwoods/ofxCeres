#pragma once

#include "AbstractCalibrationPointSet.h"

namespace Data {
	template<typename CaptureType, bool AllowMultipleSelection = true>
	class CalibrationPointSet : public AbstractCalibrationPointSet {
	public:
		vector<shared_ptr<CaptureType>> getVectorTyped(vector<shared_ptr<AbstractCalibrationPoint>> vectorUntyped) const {
			vector<shared_ptr<CaptureType>> vectorTyped;

			for (auto captureUntyped : vectorUntyped) {
				auto capture = dynamic_pointer_cast<CaptureType>(captureUntyped);
				vectorTyped.push_back(capture);
			}
			return vectorTyped;
		}

		vector<shared_ptr<CaptureType>> getSelection() const {
			return getVectorTyped(getSelectionUntyped());
		}

		vector<shared_ptr<CaptureType>> getAllCaptures() const {
			return getVectorTyped(getAllCapturesUntyped());
		}

		shared_ptr<AbstractCalibrationPoint> makeEmpty() const override {
			return make_shared<CaptureType>();
		}

		virtual std::string getTypeName() const override
		{
			return "CaptureSet";
		}
	protected:
		bool getIsMultipleSelectionAllowed() override {
			return AllowMultipleSelection;
		}
	};
}