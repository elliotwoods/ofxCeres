#pragma once

#include <type_traits>
#include <glm/glm.hpp>
#include <vector>
#include <cstring> // for memcpy

/*
This class implements a storage type for a Array2D of elements.
It doesn't implement any mathematics but it is useful for storing 2D array of elements (e.g. A pixel Array2D where every value is a ray)
*/

namespace ofxCeres {
	namespace Models {
        class BaseArray2DIterator {
        public:
            virtual size_t i() const = 0;
            virtual size_t j() const = 0;
            virtual size_t idx() const = 0;

            virtual float x() const = 0;
            virtual float y() const = 0;
        };

        template<typename T>
        struct Array2D {
            class Iterator : public BaseArray2DIterator {
            public:
                Iterator(Array2D* Array2D, T* data)
                    : Array2D(Array2D), data(data) {
                }

                // range-for requirements
                bool operator!=(const Iterator& other) const { return this->data != other.data; }
                Iterator& operator++() { ++this->data; return *this; }
                Iterator& operator*() { return *this; }     // so `auto it : Array2D` gives an Iterator copy
                Iterator* operator->() { return this; }     // enable it->value() / it->i() / it->j()

                // accessors used in your code
                T& value() { return *data; }
                size_t i() const override { return this->idx() % this->Array2D->stride; }
                size_t j() const override { return this->idx() / this->Array2D->stride; }

                // NOTE: compute against Array2D->data.data() instead of cached _begin
                size_t idx() const override {
                    return static_cast<size_t>(this->data - this->Array2D->data.data());
                }

                float x() const override { return (float)this->i() / (float)(this->Array2D->width() - 1); }
                float y() const override { return (float)this->j() / (float)(this->Array2D->height() - 1); }
            protected:
                Array2D* Array2D = nullptr;
                T* data = nullptr;
            };

            class ConstIterator : public BaseArray2DIterator {
            public:
                ConstIterator(const Array2D* Array2D, const T* data)
                    : Array2D(Array2D), data(data) {
                }

                // range-for requirements
                bool operator!=(const ConstIterator& other) const { return this->data != other.data; }
                ConstIterator& operator++() { ++this->data; return *this; }
                const ConstIterator& operator*() const { return *this; } // `auto it : const Array2D` gives a ConstIterator copy
                const ConstIterator* operator->() const { return this; } // enable it->value() / it->i() / it->j()

                // accessors
                const T& value() const { return *data; }
                size_t i() const override { return this->idx() % this->Array2D->stride; }
                size_t j() const override { return this->idx() / this->Array2D->stride; }

                // NOTE: compute against Array2D->data.data() instead of cached _begin
                size_t idx() const override {
                    return static_cast<size_t>(this->data - this->Array2D->data.data());
                }

                float x() const override { return (float)this->i() / (float)(this->Array2D->width() - 1); }
                float y() const override { return (float)this->j() / (float)(this->Array2D->height() - 1); }
            protected:
                const Array2D* Array2D = nullptr;
                const T* data = nullptr;
            };

            void allocate(size_t width, size_t height) {
                this->data.resize(width * height);
                this->stride = width;
                this->size = width * height;
                // no cached pointers to maintain
            }

            void set(const T& value) {
                for (auto it : *this) {
                    it.value() = value;
                }
            }

            void set(const T* values) {
                std::memcpy(this->data.data(), values, this->size * sizeof(T));
            }

            void clear() {
                this->data.clear();
                this->stride = 0;
                this->size = 0;
                // no cached pointers to clear
            }

            T& at(size_t i, size_t j) {
                return this->data.at(i + this->stride * j);
            }

            const T& at(size_t i, size_t j) const {
                return this->data.at(i + this->stride * j);
            }

            Iterator begin() { return Iterator(this, this->data.data()); }
            Iterator end() { return Iterator(this, this->data.data() + this->size); }

            ConstIterator begin() const { return ConstIterator(this, this->data.data()); }
            ConstIterator end()   const { return ConstIterator(this, this->data.data() + this->size); }

            size_t width() const { return this->stride; }
            size_t height() const { return this->stride ? (this->size / this->stride) : 0; }

            T& operator[](const BaseArray2DIterator& it) {
                const size_t idx = it.idx();
                return this->data.at(idx);
            }

            const T& operator[](const BaseArray2DIterator& it) const {
                const size_t idx = it.idx();
                return this->data.at(idx);
            }

            // Element-wise converting cast: Array2D<U> out = (Array2D<U>) in;
            template<typename U>
            explicit operator Array2D<U>() const {
                Array2D<U> out;
                out.allocate(this->width(), this->height());
                for (size_t k = 0; k < this->size; ++k) {
                    out.data[k] = static_cast<U>(this->data[k]);
                }
                return out;
            }

            void serialize(nlohmann::json& json)
            {
                json = nlohmann::json::object();
                json["width"] = this->width();
                json["height"] = this->height();

                auto& jsonDataArray = json["data"];
                jsonDataArray = nlohmann::json::array();

                for (size_t k = 0; k < this->size; ++k) {
                    jsonDataArray.push_back(elementToJson(this->data[k]));
                }
            }

            void deserialize(const nlohmann::json& json)
            {
                if (!json.contains("width")
                    || !json.contains("height")
                    || !json.contains("data")) {
                    return;
                }

                const size_t w = json.at("width").get<size_t>();
                const size_t h = json.at("height").get<size_t>();
                this->allocate(w, h);

                const auto& arr = json.at("data");
                if (!arr.is_array()) {
                    return;
                }
                if (arr.size() != this->size) {
                    throw std::runtime_error("Array2D<T>::deserialize: 'data' length does not match width*height");
                }

                for (size_t k = 0; k < this->size; ++k) {
                    this->jsonToElement(this->data[k], arr.at(k));
                }
            }

            std::vector<T> data;

            size_t stride = 0;
            size_t size = 0;

        private:
            // --- SFINAE detection for member serialize/deserialize on T ---
            template<typename U, typename = void>
            struct has_member_serialize : std::false_type {};
            template<typename U>
            struct has_member_serialize<U, std::void_t<
                decltype(std::declval<U&>().serialize(std::declval<nlohmann::json&>()))>> : std::true_type {};

            template<typename U, typename = void>
            struct has_member_deserialize : std::false_type {};
            template<typename U>
            struct has_member_deserialize<U, std::void_t<
                decltype(std::declval<U&>().deserialize(std::declval<const nlohmann::json&>()))>> : std::true_type {};

            template<typename U> struct dependent_false : std::false_type {};

            // --- Element <-> json helpers ---
            static nlohmann::json elementToJson(const T& v) {
                if constexpr (has_member_serialize<T>::value && has_member_deserialize<T>::value) {
                    nlohmann::json jElem;
                    const_cast<T&>(v).serialize(jElem);
                    return jElem;
                }
                else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                    return nlohmann::json(v);
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return nlohmann::json(v);
                }
                else if constexpr (std::is_same_v<T, glm::vec2>) {
                    return nlohmann::json::array({ v.x, v.y });
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    return nlohmann::json::array({ v.x, v.y, v.z });
                }
                else {
                    static_assert(dependent_false<T>::value,
                        "Array2D<T>::serialize: T has no serialize()/deserialize() and no built-in serializer here.");
                }
            }

            static void jsonToElement(T& out, const nlohmann::json& j) {
                if constexpr (has_member_serialize<T>::value && has_member_deserialize<T>::value) {
                    out.deserialize(j);
                }
                else if constexpr (std::is_same_v<T, float>) {
                    out = j.get<float>();
                }
                else if constexpr (std::is_same_v<T, double>) {
                    out = j.get<double>();
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    out = j.get<std::string>();
                }
                else if constexpr (std::is_same_v<T, glm::vec2>) {
                    if (j.is_array() && j.size() >= 2) {
                        out.x = static_cast<float>(j.at(0).get<double>());
                        out.y = static_cast<float>(j.at(1).get<double>());
                    }
                    else {
                        out.x = static_cast<float>(j.at("x").get<double>());
                        out.y = static_cast<float>(j.at("y").get<double>());
                    }
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    if (j.is_array() && j.size() >= 3) {
                        out.x = static_cast<float>(j.at(0).get<double>());
                        out.y = static_cast<float>(j.at(1).get<double>());
                        out.z = static_cast<float>(j.at(2).get<double>());
                    }
                    else {
                        out.x = static_cast<float>(j.at("x").get<double>());
                        out.y = static_cast<float>(j.at("y").get<double>());
                        out.z = static_cast<float>(j.at("z").get<double>());
                    }
                }
                else {
                    static_assert(dependent_false<T>::value,
                        "Array2D<T>::deserialize: T has no serialize()/deserialize() and no built-in deserializer here.");
                }
            }
        };
	}
}