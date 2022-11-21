#pragma once

#include "Base.h"

#include "Models/MeshBoundary.h"

namespace Elements {
	namespace Boundaries {
		class Mesh : public Base {
		public:
			Mesh();

			void initLine(float width);
			void initCurve(float radius, float angle, size_t resolution = 8);

			string getTypeName() const override;
			string getGlyph() const override;

			template<typename T>
			Models::MeshBoundary_<T>
				getModel() {
				Models::MeshBoundary_<T> model;

				const glm::vec2 center(Elements::Base::parameters.position.x.get()
					, Elements::Base::parameters.position.y.get());

				for (auto vertex = this->vertices.begin()
					; vertex != this->vertices.end()
					; vertex++)
				{
					model.vertices.emplace(vertex->first + center.x, (T)vertex->second + (T)center.y);
				}
				model.exitVsEntranceIOR = Boundaries::Base::parameters.exitIOR.get()
					/ Boundaries::Base::parameters.entranceIOR.get();
				return model;
			}
		protected:
			void draw();
			std::map<float, float> vertices;
		};
	}
}