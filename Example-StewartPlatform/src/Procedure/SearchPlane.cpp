#include "pch_ofApp.h"
#include "SearchPlane.h"
#include "kdtree/kdtree.h"

#include <thread>
#include <future>

namespace Procedure {
	//----------
	void
		SearchPlane::draw()
	{
		this->preview.drawVertices();
	}

	//----------
	void
		SearchPlane::perform(Data::StewartPlatform& stewartPlatform)
	{
		glm::vec3 centerTranslate{
			stewartPlatform.transform.translate.x.get()
			, stewartPlatform.transform.translate.y.get()
			, stewartPlatform.transform.translate.z.get()
		};

		glm::vec3 centerRotate{
			stewartPlatform.transform.rotate.x.get()
			, stewartPlatform.transform.rotate.y.get()
			, stewartPlatform.transform.rotate.z.get()
		};

		this->validPositions.clear();


		this->performAtPoint(stewartPlatform
			, centerTranslate
			, this->parameters.initialStepSize.get()
			, this->parameters.stepDepth.get());

		// move back to start
		stewartPlatform.isValidTransform(centerTranslate, centerRotate);

		// prepare the preview
		{
			// find the limits
			float zMin = std::numeric_limits<float>::max();
			float zMax = std::numeric_limits<float>::min();
			for (const auto& point : this->validPositions) {
				if (point.z > zMax) {
					zMax = point.z;
				}
				if (point.z < zMin) {
					zMin = point.z;
				}
			}

			this->preview.clear();
			this->preview.addVertices(this->validPositions);

			for (const auto& point : this->validPositions) {
				ofColor color(200, 100, 100);
				color.setHueAngle(ofMap(point.z, zMin, zMax, 0, 360));
				this->preview.addColor(color);
			}
		}
	}

	//----------
	const std::vector<glm::vec3>&
		SearchPlane::getPositions() const
	{
		return this->validPositions;
	}

	//----------
	void
		SearchPlane::performAtPoint(Data::StewartPlatform& stewartPlatform
			, const glm::vec3& position
			, float stepSize
			, int remainingIterations)
	{
		remainingIterations--;

		auto thisIsValid = stewartPlatform.isValidTransform(position, glm::vec3(0, 0, 0));
		if (thisIsValid) {
			this->validPositions.push_back(position);
		}

		// Iterate
		if (remainingIterations-- > 0) {
			std::vector<glm::vec3> offsets{
				glm::vec3(-1, 0, 0) * stepSize
				, glm::vec3(1, 0, 0) * stepSize
				, glm::vec3(0, -1, 0) * stepSize
				, glm::vec3(0, +1, 0) * stepSize
				, glm::vec3(0, 0, -1) * stepSize
				, glm::vec3(0, 0, +1) * stepSize
			};

			for (const auto& offset : offsets) {
				auto thatIsValid = stewartPlatform.isValidTransform(position + offset, glm::vec3(0, 0, 0));

				if (thatIsValid) {
					this->validPositions.push_back(position + offset);
				}

				if (thisIsValid != thatIsValid) {
					this->performAtPoint(stewartPlatform
						, position + offset / 2
						, stepSize / 2
						, remainingIterations);
				}
				else if(thisIsValid && thatIsValid) {
					this->performAtPoint(stewartPlatform
						, position + offset * 1.5
						, stepSize
						, remainingIterations);
				}
			}
		}
	}
}
