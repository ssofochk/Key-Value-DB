#pragma once

#include <string>
#include <type_traits>

template <typename T>
void ToBytes(char*& buf, const T& value);

template <typename T>
    requires(std::is_fundamental_v<T> && !std::is_same_v<T, void> && !std::is_same_v<T, std::nullptr_t>)
void ToBytes(char*& buf, const T& value) {
    std::memcpy(buf, &value, sizeof(T));
    buf += sizeof(T);
}

inline void ToBytes(char*& buf, const std::string& value) {
    size_t size = value.size();
    ToBytes(buf, size);

    std::memcpy(buf, value.data(), size);
    buf += size;
}