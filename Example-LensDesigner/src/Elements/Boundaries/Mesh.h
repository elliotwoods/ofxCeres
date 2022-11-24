#pragma once

#include "Base.h"

#include "Models/MeshBoundary.h"

namespace Elements {
	namespace Boundaries {
		class Mesh : public Base {
		public:
			Mesh();

			void initLine(float width);
			void initCurve(float radius, float angle);
			void initDefaultLine();
			void initDefaultCurve();

			void doubleMeshResolution();

			string getTypeName() const override;
			string getGlyph() const override;

			template<typename T>
			shared_ptr<Models::MeshBoundary_<T>>
				getModel() const
			{
				auto model = make_shared<Models::MeshBoundary_<T>>();

				const glm::vec2 center(Elements::Base::parameters.position.x.get()
					, Elements::Base::parameters.position.y.get());

				for (auto vertex = this->vertices.begin()
					; vertex != this->vertices.end()
					; vertex++)
				{
					model->vertices.emplace(vertex->first + center.x, (T)vertex->second + (T)center.y);
				}
				model->exitVsEntranceIOR = Boundaries::Base::parameters.exitIOR.get()
					/ Boundaries::Base::parameters.entranceIOR.get();
				model->configureParameters = [this, center](ceres::Problem& problem, double* parameters) {
					auto upperBound = center.y + this->parameters.maxThickness.get();
					auto lowerBound = center.y;
					for (size_t i = 0; i < this->vertices.size(); i++) {
						problem.SetParameterUpperBound(parameters, i, upperBound);
						problem.SetParameterLowerBound(parameters, i, lowerBound);
					}
					if (this->parameters.fixFinalVertex) {
						auto i = this->vertices.size() - 1;
						problem.SetParameterUpperBound(parameters, i, lowerBound + 0.0001);
						problem.SetParameterLowerBound(parameters, i, lowerBound);
					}
				};
				return model;
			}

			template<typename T>
			void
				setModel(shared_ptr<Models::MeshBoundary_<T>> model)
			{
				const glm::vec2 center(Elements::Base::parameters.position.x.get()
					, Elements::Base::parameters.position.y.get());
				this->vertices.clear();
				for (const auto & vertex : model->vertices) {
					this->vertices.emplace(vertex.first - center.x, (float)vertex.second - center.y);
				}
			}

			shared_ptr<Models::OpticalElement_<float>> getOpticalModelUntyped() const override;
			void setOpticalModelUntyped(shared_ptr<Models::OpticalElement_<float>>) override;

			struct : ofParameterGroup {
				struct : ofParameterGroup {
					struct : ofParameterGroup {
						ofParameter<float> radius{ "Radius", 2, -2, 2 };
						ofParameter<float> angleWidth{ "Angle width (pi)", 0.1, 0, 2 };
						ofParameter<int> resolution{ "Resolution", 2, 2, 100 };
						PARAM_DECLARE("Curve", radius, angleWidth, resolution)
					} curve;
					struct : ofParameterGroup {
						ofParameter<float> width{ "Width", 0.04, 0, 1 };
						PARAM_DECLARE("Line", width)
					} line;
					PARAM_DECLARE("Defaults", curve, line);
				} defaults;
				ofParameter<float> maxThickness{ "Max thickness", 0.03f, 0.0f, 1.0f };
				ofParameter<bool> fixFinalVertex{ "Fix final vertex", true };
				PARAM_DECLARE("Mesh", defaults, maxThickness, fixFinalVertex);
			} parameters;

			void exportSVG();
		protected:
			void draw();
			std::map<float, float> vertices;
		};
	}
}