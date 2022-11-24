#include "pch_ofApp.h"
#include "Flat.h"

namespace Elements {
	namespace Boundaries {
		//----------
		Flat::Flat()
		{
			this->manageParameters(this->parameters);

			this->onDrawObjectSpace += [this]() {
				const auto h = 0.01f;

				ofPushMatrix();
				{
					ofRotateZDeg(this->parameters.angle);

					ofRotateXDeg(90);
					auto width = this->parameters.width;

					ofPushStyle();
					glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
					{
						glEnable(GL_CULL_FACE);

						glCullFace(GL_BACK);
						{
							ofNoFill();
							ofSetLineWidth(1.0f);
							ofDrawRectangle(-width / 2.0, 0, width, h);

							ofFill();
							ofSetColor(127.0f / Boundaries::Base::parameters.entranceIOR.get());
							ofDrawRectangle(-width / 2.0, 0, width, h);
						}

						glCullFace(GL_FRONT);
						{
							ofNoFill();
							ofSetLineWidth(1.0f);
							ofDrawRectangle(-width / 2.0, 0, width, h);

							ofFill();
							ofSetColor(127.0f / Boundaries::Base::parameters.exitIOR.get());
							ofDrawRectangle(-width / 2.0, 0, width, h);
						}

					}
					glPopAttrib();
					ofPopStyle();
				}
				ofPopMatrix();
			};
		}

		//----------
		string
			Flat::getTypeName() const
		{
			return "Flat";
		}

		//----------
		string
			Flat::getGlyph() const
		{
			return u8"\uf7a4";
		}

		//----------
		shared_ptr<Models::OpticalElement_<float>>
			Flat::getOpticalModelUntyped() const
		{
			return this->getModel<float>();
		}

		//----------
		void
			Flat::setOpticalModelUntyped(shared_ptr<Models::OpticalElement_<float>> model)
		{
			auto typedModel = dynamic_pointer_cast<Models::FlatBoundary_<float>>(model);
			if (!typedModel) {
				throw(ofxCeres::Exception("Type cast exception"));
			}
			return this->setModel(typedModel);
		}
	}
}