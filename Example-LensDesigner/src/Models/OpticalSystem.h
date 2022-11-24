#pragma once

#include "OpticalElement.h"
#include "Ray.h"
#include <vector>
#include "ofxCeres.h"

#include "FlatBoundary.h"
#include "MeshBoundary.h"

namespace Models {
	template<typename T>
	struct OpticalSystem_ {
		std::vector<shared_ptr<OpticalElement_<T>>> opticalElements;

		bool
			interact(RayChain_<T>& rayChain) const
		{
			for (const auto& opticalElement : this->opticalElements) {
				auto result = opticalElement->interact(rayChain.back());
				if (!result) {
					return false;
				}
				rayChain.push_back(*result);
			}
			return true;
		}

		template<typename T2>
		OpticalSystem_<T2>
			castTo() const
		{
			OpticalSystem_<T2> castOpticalSystem;
			for (auto opticalElement : this->opticalElements) {
				{
					auto typedOpticalElement = dynamic_pointer_cast<FlatBoundary_<T>>(opticalElement);
					if (typedOpticalElement) {
						castOpticalSystem.opticalElements.push_back(typedOpticalElement->castTo<T2>());
						continue;
					}
				}

				{
					auto typedOpticalElement = dynamic_pointer_cast<MeshBoundary_<T>>(opticalElement);
					if (typedOpticalElement) {
						castOpticalSystem.opticalElements.push_back(typedOpticalElement->castTo<T2>());
						continue;
					}
				}

				throw(ofxCeres::Exception("Can't cast optical element"));
			}
			return castOpticalSystem;
		}

		void
			setParameters(T const* const* allParameters)
		{
			auto parametersMover = allParameters;
			for (auto opticalElement : this->opticalElements) {
				if (opticalElement->getParameterCount() > 0) {
					opticalElement->setParameters(*parametersMover++);
				}
			}
		}
	};
	typedef OpticalSystem_<float> OpticalSystem;
}