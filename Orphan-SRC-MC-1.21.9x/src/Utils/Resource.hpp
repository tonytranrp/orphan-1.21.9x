#pragma once
//
// Created by vastrakai on 6/29/2024.
//
#include <string>
#include <span>
#include <cstddef>


struct Resource {
    Resource() = default;
    Resource(const char* begin, const char* end) : _begin(begin), _end(end) {}
    [[nodiscard]] const char* data() const { return _begin; }
    [[nodiscard]] void* data2() const { return (void*)_begin; }
    [[nodiscard]] const char* begin() const { return _begin; }
    [[nodiscard]] const char* end() const { return _end; }
    [[nodiscard]] size_t size() const { return size_t(_end - _begin); }
    [[nodiscard]] std::string_view str() const { return std::string_view{this->data(), this->size()}; }
    [[nodiscard]] std::span<const std::byte> bytes() const {
        return {
                reinterpret_cast<const std::byte*>(this->begin()),
                reinterpret_cast<const std::byte*>(this->end())
        };
    }
private:
    const char* _begin;
    const char* _end;
};

#define LOAD_RESOURCE(x)                                     \
    extern "C" const unsigned char _binary_resources_##x[]; \
    extern "C" const size_t _binary_resources_##x##_size;

#define GET_RESOURCE(x) Resource{                              \
    reinterpret_cast<const char*>(_binary_resources_##x),     \
    reinterpret_cast<const char*>(_binary_resources_##x) +    \
        _binary_resources_##x##_size                          \
}
