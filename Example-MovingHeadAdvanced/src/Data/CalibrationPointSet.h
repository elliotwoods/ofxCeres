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

		void sortByDate()
		{
			this->sortBy([](shared_ptr<CaptureType> capture) {
				auto millis = std::chrono::time_point_cast<std::chrono::milliseconds>(capture->timestamp.get());
				return (float)millis.time_since_epoch().count();
				});
		}

		void sortBy(function<float(shared_ptr<CaptureType>)> sortFunction) {
			this->sortBy([&](const shared_ptr<CaptureType>& a, const shared_ptr<CaptureType>& b) {
					return sortFunction(a) < sortFunction(b);
				});
			this->onChange.notifyListeners();
			this->viewDirty = true;
		}

		void sortBy(function<bool(shared_ptr<CaptureType>, shared_ptr<CaptureType>)> lessThanFunction) {
			sort(this->captures.begin(), this->captures.end(), [&lessThanFunction](shared_ptr<AbstractCalibrationPoint> a, shared_ptr<AbstractCalibrationPoint> b) {
				auto aTyped = dynamic_pointer_cast<CaptureType>(a);
				auto bTyped = dynamic_pointer_cast<CaptureType>(b);
				if (aTyped && bTyped) {
					return lessThanFunction(aTyped, bTyped);
				}
				else {
					return false;
				}
				});
			this->onChange.notifyListeners();
			this->viewDirty = true;
		}

	protected:
		bool getIsMultipleSelectionAllowed() override {
			return AllowMultipleSelection;
		}
	};
}