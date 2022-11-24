#pragma once

namespace Models {
	template<typename T>
	struct Ray_ {
		glm::tvec2<T> s;
		glm::tvec2<T> t;

		T
			distanceTo(const glm::tvec2<T>& point)
		{
			auto closestPointOnLine = glm::dot(point - s, t) * t + s;
			auto distance = glm::distance(point, closestPointOnLine);
			return distance;
		}

		template<typename T2>
		Ray_<T2> castTo()
		{
			Ray_<T2> instance2;
			{
				instance2.s = (glm::tvec2<T2>) this->s;
				instance2.t = (glm::tvec2<T2>) this->t;
			}
			return instance2;
		}
	};

	typedef Ray_<float> Ray;

	template<typename T>
	struct RayChain_ : std::vector<Ray_<T>> {

	};

	typedef RayChain_<float> RayChain;
}