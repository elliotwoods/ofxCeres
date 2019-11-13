#pragma once

#include <ceres/ceres.h>
namespace std {
	template<typename T, int N>
	struct numeric_limits<ceres::Jet<T, N> > {
		static const bool is_signed = true;
		static const bool is_integer = false;
		static const bool is_iec559 = true;
	};
}

#include <glm/glm.hpp>