#pragma once

#include <type_traits>
#include <glm/glm.hpp>
#include <vector>
#include <cstring> // memcpy

/*
This class implements a storage type for a 1D array of elements.
It mirrors Array2D’s style (iterators, JSON I/O, SFINAE element (de)serialization)
but for a linear container.
*/

namespace ofxCeres {
    namespace Models {

        class BaseArray1DIterator {
        public:
            virtual size_t idx() const = 0;
            virtual ~BaseArray1DIterator() = default;
        };

        template<typename T>
        struct Array1D {
            // -------- iterators --------
            class Iterator : public BaseArray1DIterator {
            public:
                Iterator(Array1D* owner, T* ptr) : owner(owner), ptr(ptr) {}
                bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
                Iterator& operator++() { ++ptr; return *this; }
                Iterator& operator*() { return *this; }     // allows `auto it : array`
                Iterator* operator->() { return this; }     // it->value()

                T& value() { return *ptr; }
                size_t idx() const override { return static_cast<size_t>(ptr - owner->data.data()); }
            private:
                Array1D* owner = nullptr;
                T* ptr = nullptr;
            };

            class ConstIterator : public BaseArray1DIterator {
            public:
                ConstIterator(const Array1D* owner, const T* ptr) : owner(owner), ptr(ptr) {}
                bool operator!=(const ConstIterator& other) const { return ptr != other.ptr; }
                ConstIterator& operator++() { ++ptr; return *this; }
                const ConstIterator& operator*() const { return *this; }
                const ConstIterator* operator->() const { return this; }

                const T& value() const { return *ptr; }
                size_t idx() const override { return static_cast<size_t>(ptr - owner->data.data()); }
            private:
                const Array1D* owner = nullptr;
                const T* ptr = nullptr;
            };

            // -------- core API --------
            void allocate(size_t length) {
                data.resize(length);
                size_ = length;
            }

            void clear() {
                data.clear();
                size_ = 0;
            }

            size_t size() const { return size_; }
            bool empty() const { return size_ == 0; }

            void set(const T& value) {
                for (auto it : *this) {
                    it.value() = value;
                }
            }

            void set(const T* values) {
                std::memcpy(data.data(), values, size_ * sizeof(T));
            }

            T& at(size_t i) { return data.at(i); }
            const T& at(size_t i) const { return data.at(i); }

            T& operator[](size_t i) { return data[i]; }
            const T& operator[](size_t i) const { return data[i]; }

            // range-for
            Iterator begin() { return Iterator(this, data.data()); }
            Iterator end() { return Iterator(this, data.data() + size_); }
            ConstIterator begin() const { return ConstIterator(this, data.data()); }
            ConstIterator end()   const { return ConstIterator(this, data.data() + size_); }

            // converting cast: Array1D<U> out = (Array1D<U>) in;
            template<typename U>
            explicit operator Array1D<U>() const {
                Array1D<U> out;
                out.allocate(size_);
                for (size_t k = 0; k < size_; ++k) {
                    out.data[k] = static_cast<U>(data[k]);
                }
                return out;
            }

            // -------- JSON I/O --------
            void serialize(nlohmann::json& json) {
                json = nlohmann::json::object();
                json["length"] = size_;

                auto& arr = json["data"];
                arr = nlohmann::json::array();
                arr.reserve(size_);

                for (size_t k = 0; k < size_; ++k) {
                    arr.push_back(elementToJson(data[k]));
                }
            }

            void deserialize(const nlohmann::json& json) {
                if (!json.contains("length") || !json.contains("data")) {
                    // be permissive: just clear if malformed
                    clear();
                    return;
                }

                const size_t n = json.at("length").get<size_t>();
                allocate(n);

                const auto& arr = json.at("data");
                if (!arr.is_array()) {
                    clear();
                    return;
                }
                if (arr.size() != size_) {
                    throw std::runtime_error("Array1D<T>::deserialize: 'data' length does not match 'length'");
                }

                for (size_t k = 0; k < size_; ++k) {
                    jsonToElement(data[k], arr.at(k));
                }
            }

            std::vector<T> data;

        private:
            size_t size_ = 0;

            // --- SFINAE detection for member serialize/deserialize on T ---
            template<typename U, typename = void>
            struct has_member_serialize : std::false_type {};
            template<typename U>
            struct has_member_serialize<U, std::void_t<
                decltype(std::declval<U&>().serialize(std::declval<nlohmann::json&>()))
                >> : std::true_type {};

            template<typename U, typename = void>
            struct has_member_deserialize : std::false_type {};
            template<typename U>
            struct has_member_deserialize<U, std::void_t<
                decltype(std::declval<U&>().deserialize(std::declval<const nlohmann::json&>()))
                >> : std::true_type {};

            template<typename U> struct dependent_false : std::false_type {};

            // --- Element <-> json helpers ---
            static nlohmann::json elementToJson(const T& v) {
                if constexpr (has_member_serialize<T>::value && has_member_deserialize<T>::value) {
                    nlohmann::json jElem;
                    const_cast<T&>(v).serialize(jElem); // many user types have non-const serialize
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
                        "Array1D<T>::serialize: T has no serialize()/deserialize() and no built-in serializer here.");
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
                        "Array1D<T>::deserialize: T has no serialize()/deserialize() and no built-in deserializer here.");
                }
            }
        };

    } // namespace Models
} // namespace ofxCeres
