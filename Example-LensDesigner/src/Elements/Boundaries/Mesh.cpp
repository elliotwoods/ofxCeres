#include "pch_ofApp.h"
#include "Mesh.h"

namespace Elements {
	namespace Boundaries {
		//----------
		Mesh::Mesh()
		{
			this->manageParameters(this->parameters);

			this->onDrawObjectSpace += [this]() {
				this->draw();
			};

			this->onPopulateInspector += [this](ofxCvGui::InspectArguments& args) {
				auto inspector = args.inspector;
				inspector->addButton("Init default curve", [this]() {
					this->initDefaultCurve();
					});
				inspector->addButton("Init default line", [this]() {
					this->initDefaultLine();
					});
				inspector->addButton("Double mesh resolution", [this]() {
					this->doubleMeshResolution();
					});
				inspector->addButton("Export SVG...", [this]() {
					this->exportSVG();
					});
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
			Mesh::initCurve(float radius, float angle)
		{
			this->vertices.clear();

			const auto& resolution = this->parameters.defaults.curve.resolution.get();
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
		void
			Mesh::initDefaultLine()
		{
			this->initLine(this->parameters.defaults.line.width);
		}

		//----------
		void
			Mesh::initDefaultCurve()
		{
			this->initCurve(this->parameters.defaults.curve.radius
				, this->parameters.defaults.curve.angleWidth);
		}

		//----------
		void
			Mesh::doubleMeshResolution()
		{
			map<float, float> newVertices;
			for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
				auto nextVertex = it;
				nextVertex++;
				if (nextVertex == this->vertices.end()) {
					break;
				}
				newVertices.emplace((it->first + nextVertex->first) / 2.0f, (it->second + nextVertex->second) / 2.0f);
			}
			for (const auto& newVertex : newVertices) {
				this->vertices.emplace(newVertex);
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
		shared_ptr<Models::OpticalElement_<float>>
			Mesh::getOpticalModelUntyped() const
		{
			return this->getModel<float>();
		}

		//----------
		void
			Mesh::setOpticalModelUntyped(shared_ptr<Models::OpticalElement_<float>> model)
		{
			auto typedModel = dynamic_pointer_cast<Models::MeshBoundary_<float>>(model);
			if (!typedModel) {
				throw(ofxCeres::Exception("Type cast exception"));
			}
			return this->setModel(typedModel);
		}

		//----------
		void
			Mesh::exportSVG()
		{
			auto result = ofSystemSaveDialog("output.svg", "Save SVG");
			if (!result.bSuccess) {
				return;
			}

			ofSetBackgroundAuto(false);
			ofBeginSaveScreenAsSVG(result.filePath);
			{
				ofTranslate(this->getPosition());
				ofDrawLine({ -1, 0, 0 }, { 1, 0, 0 });
				ofDrawLine({ 0, -1, 0 }, { 0, 1, 0 });

				ofPushMatrix();
				{
					ofScale(1000.f, -1000.0f, 1000.0f);

					ofPolyline line;

					auto vertex = this->vertices.begin();
					line.addVertex({ vertex->first, vertex->second, 0.0f });

					vertex++;
					for (; vertex != this->vertices.end(); vertex++) {
						line.lineTo({ vertex->first, vertex->second, 0.0f });
					}
					line.close();

					line.draw();
				}
				ofPopMatrix();
			}
			ofEndSaveScreenAsSVG();
			ofSetBackgroundAuto(true);
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

			const float h = 0.01f;

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

			// Draw upper and lower bounds
			{
				auto top = this->parameters.maxThickness.get();
				auto bottom = 0;
				auto left = this->vertices.begin()->first;
				auto right = this->vertices.rbegin()->first;

				{
					ofMesh line;
					line.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINE_LOOP);
					line.addVertex({ left, top, 0 });
					line.addVertex({ right, top, 0 });
					line.addVertex({ right, top, h });
					line.addVertex({ left, top, h });
					line.draw();
				}

				{
					ofMesh line;
					line.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINE_LOOP);
					line.addVertex({ left, bottom, 0 });
					line.addVertex({ right, bottom, 0 });
					line.addVertex({ right, bottom, h });
					line.addVertex({ left, bottom, h });
					line.draw();
				}
			}
		}
	}
}