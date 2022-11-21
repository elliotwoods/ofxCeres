#include "pch_ofApp.h"
#include "Flat.h"

namespace Elements {
	namespace Boundaries {
		//----------
		Flat::Flat()
		{
			this->manageParameters(this->parameters);

			this->onDrawObjectSpace += [this]() {
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
							ofDrawRectangle(-width / 2.0, 0, width, 0.1);

							ofFill();
							ofSetColor(127.0f / Boundaries::Base::parameters.entranceIOR.get());
							ofDrawRectangle(-width / 2.0, 0, width, 0.1);
						}

						glCullFace(GL_FRONT);
						{
							ofNoFill();
							ofSetLineWidth(1.0f);
							ofDrawRectangle(-width / 2.0, 0, width, 0.1);

							ofFill();
							ofSetColor(127.0f / Boundaries::Base::parameters.exitIOR.get());
							ofDrawRectangle(-width / 2.0, 0, width, 0.1);
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
	}
}