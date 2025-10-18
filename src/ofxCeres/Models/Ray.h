#pragma once

#include "Base.h"
#include <glm/glm.hpp>
#include <vector>

#include "pch_ofxCeres.h"
#include "ofxCeres/VectorMath/VectorMath.h"

namespace ofxCeres {
	namespace Models {
		template<typename T>
		struct Ray {
			glm::tvec3<T> s;
			glm::tvec3<T> t;

			template<typename T2>
			Ray<T2>
				castTo() const
			{
				Ray<T2> castRay;
				{
					castRay.s = (glm::tvec3<T2>) this->s;
					castRay.t = (glm::tvec3<T2>) this->t;
				}
				return castRay;
			}

			void setStart(const glm::tvec3<T>& start) {
				this->s = start;
			}

			void setEnd(const glm::tvec3<T>& end) {
				this->t = end - this->s;
			}

			glm::tvec3<T> getStart() const
			{
				return this->s;
			}

			glm::tvec3<T> getEnd() const
			{
				return this->s + this->t;
			}

			T getLength() const {
				return VectorMath::length(this->t);
			}

			void setLength(T length) {
				this->t *= length / this->getLength();
			}

			Ray intersect(const Ray& other) {
				const glm::tvec3<T> p1(this->s), p2(this->s + this->t), p3(other.s), p4(other.s + other.t);
				const float EPS(1.0E-15);

				glm::vec3 p13, p43, p21;
				float d1343, d4321, d1321, d4343, d2121;
				float numer, denom;

				p13.x = p1.x - p3.x;
				p13.y = p1.y - p3.y;
				p13.z = p1.z - p3.z;
				p43.x = p4.x - p3.x;
				p43.y = p4.y - p3.y;
				p43.z = p4.z - p3.z;
				if (fabs(p43.x) < EPS && fabs(p43.y) < EPS && fabs(p43.z) < EPS)
					return(Ray());
				p21.x = p2.x - p1.x;
				p21.y = p2.y - p1.y;
				p21.z = p2.z - p1.z;
				if (fabs(p21.x) < EPS && fabs(p21.y) < EPS && fabs(p21.z) < EPS)
					return(Ray());

				d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
				d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
				d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
				d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
				d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

				denom = d2121 * d4343 - d4321 * d4321;
				if (fabs(denom) < EPS)
					return(Ray());
				numer = d1343 * d4321 - d1321 * d4343;

				float ma = numer / denom;
				float mb = (d1343 + d4321 * (ma)) / d4343;

				glm::vec3 s;
				glm::vec3 t;

				s.x = p1.x + ma * p21.x;
				s.y = p1.y + ma * p21.y;
				s.z = p1.z + ma * p21.z;
				t.x = p3.x + mb * p43.x;
				t.y = p3.y + mb * p43.y;
				t.z = p3.z + mb * p43.z;
				t = t - s;

				Ray intersectionRay;
				{
					intersectionRay.s = s;
					intersectionRay.t = t;
				}
				return intersectionRay;
			}

			T distanceTo(const glm::tvec3<T>& point) const {
				return VectorMath::length(VectorMath::cross(point - s, point - (s + t))) / VectorMath::length(t);
			}

			glm::tvec3<T> closestPointOnRayTo(const glm::tvec3<T>& point) const {
				return s + (t * VectorMath::dot(point - s, t) / VectorMath::length2(t));
			}

			glm::tvec3<T> getMidpoint() const {
				return s + t * 0.5;
			}

			Ray applyTransform(const glm::tmat4x4<T>& m) const {
				auto s = m * glm::tvec4<T>(this->s, 1.0f);
				auto t = (glm::tmat3x3<T>) m * this->t;

				return Ray<T>(
					{ s.x, s.y, s.z },
					{ t.x, t.y, t.z }
				);
			}

			// Element-wise converting cast: Grid<U> out = (Grid<U>) in;
			template<typename T2>
			explicit operator Ray<T2>() const {
				Ray<T2> out;
				out.s = (glm::tvec3<T2>) this->s;
				out.t = (glm::tvec3<T2>) this->t;
				return out;
			}

			// Declarations so we can specialize for T=float below
			void serialize(nlohmann::json& json);
			void deserialize(const nlohmann::json& json);
		};

		// Ray<float> JSON specialization
		template<>
		inline void Ray<float>::serialize(nlohmann::json& json)
		{
			json = nlohmann::json::object();
			json["s"] = nlohmann::json::array({ this->s.x, this->s.y, this->s.z });
			json["t"] = nlohmann::json::array({ this->t.x, this->t.y, this->t.z });
		}

		template<>
		inline void Ray<float>::deserialize(const nlohmann::json& json)
		{
			if (!json.contains("s") || !json.contains("t")) {
				throw std::runtime_error("Ray<float>::deserialize: missing keys 's' or 't'");
			}

			auto readVec3 = [](const nlohmann::json& j) -> glm::vec3 {
				glm::vec3 v;
				if (j.is_array()) {
					if (j.size() != 3) {
						throw std::runtime_error("Ray<float>::deserialize: vec3 array must have 3 elements");
					}
					v.x = j.at(0).get<float>();
					v.y = j.at(1).get<float>();
					v.z = j.at(2).get<float>();
				}
				else {
					// Accept object form too: { "x":..., "y":..., "z":... }
					v.x = j.at("x").get<float>();
					v.y = j.at("y").get<float>();
					v.z = j.at("z").get<float>();
				}
				return v;
				};

			this->s = readVec3(json.at("s"));
			this->t = readVec3(json.at("t"));
		}
	}
}

