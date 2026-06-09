#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <concepts>
#include <cstddef>

#include "string_element.hpp"
#include "list_element.hpp"
#include "set_element.hpp"
#include "geo_element.hpp"

using DataBaseStoredTypes = std::variant<StringElement, ListElement, SetElement, GeoElement>;
using DeathTime = std::chrono::steady_clock::time_point;

struct DataBaseElement {
    DataBaseStoredTypes value_;
    std::optional<DeathTime> death_time_;
};

template <class Storage>
concept DataBaseStorage = requires(
    Storage storage,
    std::string key,
    typename Storage::iterator iterator
) {
    typename Storage::key_type;
    typename Storage::mapped_type;
    typename Storage::iterator;

    requires std::same_as<typename Storage::key_type, std::string>;
    requires std::same_as<typename Storage::mapped_type, DataBaseElement>;

    { storage.find(key) } -> std::same_as<typename Storage::iterator>;
    { storage.begin() } -> std::same_as<typename Storage::iterator>;
    { storage.end() } -> std::same_as<typename Storage::iterator>;

    { iterator->first } -> std::convertible_to<const std::string&>;
    { iterator->second } -> std::same_as<DataBaseElement&>;

    { storage.erase(iterator) } -> std::same_as<typename Storage::iterator>;
    { storage.clear() } -> std::same_as<void>;
    { storage.size() } -> std::convertible_to<std::size_t>;
    { storage[key] } -> std::same_as<DataBaseElement&>;
};

using DefaultStorage = std::unordered_map<std::string, DataBaseElement>;