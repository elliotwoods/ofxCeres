#include "pch_ofApp.h"
#include "SearchPlane.h"

namespace Procedure {
	//----------
	void
		SearchPlane::draw()
	{
		if (this->validPositions.empty()) {
			return;
		}

		const auto& zMin = this->validPositions.front().z;
		const auto& zMax = this->validPositions.back().z;

		ofPushStyle();
		for (const auto& point : this->validPositions) {
			ofPushMatrix();
			{
				ofTranslate(point);
				ofColor color(200, 100, 100);
				color.setHueAngle(ofMap(point.z, zMin, zMax, 0, 360));
				ofSetColor(color);
				ofDrawCircle(0, 0, 0.002);
			}
			ofPopMatrix();
		}
		ofPopStyle();
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

		auto stepSize = this->parameters.windowSize.get() * 2 / (float) this->parameters.resolution.get();
		for (float z = -this->parameters.windowSize.get(); z <= this->parameters.windowSize.get(); z += stepSize) {
			for (float y = -this->parameters.windowSize.get(); y <= this->parameters.windowSize.get(); y += stepSize) {
				for (float x = -this->parameters.windowSize.get(); x <= this->parameters.windowSize.get(); x += stepSize) {
					glm::vec3 translate{
					x + centerTranslate.x
					, y + centerTranslate.y
					, z + centerTranslate.z
					};
					if (stewartPlatform.isValidTransform(translate, centerRotate)) {
						this->validPositions.push_back(translate);
					}
				}
			}
		}

		// move back to start
		stewartPlatform.isValidTransform(centerTranslate, centerRotate);
	}

	//----------
	const std::vector<glm::vec3>&
		SearchPlane::getPositions() const
	{
		return this->validPositions;
	}
}
