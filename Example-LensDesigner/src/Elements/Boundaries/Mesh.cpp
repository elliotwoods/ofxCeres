#include "pch_ofApp.h"
#include "Mesh.h"

namespace Elements {
	namespace Boundaries {
		//----------
		Mesh::Mesh()
		{
			this->onDrawObjectSpace += [this]() {
				this->draw();
			};
		}
		
		//----------
		void
			Mesh::initLine(float width)
		{
			this->vertices.clear();
			this->vertices.emplace(-width / 2.0f, 0);
			this->vertices.emplace(width / 2.0f, 0);
		}

		//----------
		void
			Mesh::initCurve(float radius, float angle, size_t resolution)
		{

			this->vertices.clear();

			auto theta = -angle/2.0f;
			auto step = angle / (resolution - 1);

			float c;

			for (size_t i = 0; i < resolution; i++) {
				auto x = radius * sin(theta);
				auto y = radius * cos(theta) - radius;

				this->vertices.emplace(x, y);

				theta += step;
			}
		}

		//----------
		string
			Mesh::getTypeName() const
		{
			return "Mesh";
		}

		//----------
		string
			Mesh::getGlyph() const
		{
			return u8"\uf5ee";
		}

		//----------
		void
			Mesh::draw()
		{
			if (this->vertices.size() < 2) {
				return;
			}

			ofMesh triangles;
			triangles.setMode(ofPrimitiveMode::OF_PRIMITIVE_TRIANGLE_STRIP);

			ofMesh lines;
			lines.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINE_LOOP);

			ofMesh verticals;
			verticals.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINES);

			const float h = 0.1f;

			{
				{
					auto vertex = this->vertices.begin();

					// left edge
					{
						const auto& x = vertex->first;
						const auto& y = vertex->second;

						lines.addVertex({ x, y, 0 });
						lines.addVertex({ x, y, h });

						triangles.addVertex({ x, y, 0 });
						triangles.addVertex({ x, y, h });
					}

					// along top
					for (; vertex != this->vertices.end(); vertex++)
					{
						const auto& x = vertex->first;
						const auto& y = vertex->second;

						lines.addVertex({ x, y, h });
						triangles.addVertex({ x, y, 0 });
						triangles.addVertex({ x, y, h });

						verticals.addVertex({ x, y, 0 });
						verticals.addVertex({ x, y, h });

					}
				}

				// back along bottom
				for (auto rvertex = this->vertices.rbegin(); rvertex != this->vertices.rend(); rvertex++)
				{
					const auto& x = rvertex->first;
					const auto& y = rvertex->second;

					lines.addVertex({ x, y, 0 });
				}
			}

			glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
			ofPushStyle();
			{
				glEnable(GL_CULL_FACE);

				glCullFace(GL_BACK);
				{
					ofSetColor(127.0f / Boundaries::Base::parameters.exitIOR.get());
					triangles.draw();
				}

				glCullFace(GL_FRONT);
				{
					ofSetColor(127.0f / Boundaries::Base::parameters.entranceIOR.get());
					triangles.draw();
				}
			}
			glPopAttrib();
			ofPopStyle();

			lines.draw();
			verticals.draw();
		}
	}
}