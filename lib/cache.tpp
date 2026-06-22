#include <chrono>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

template <typename Type>
std::size_t ObjectMemory(const Type& value) {
    if constexpr (requires { value.capacity(); }) {
        return sizeof(Type) + value.capacity();
    } else {
        return sizeof(Type);
    }
}

template <Policies Policy, typename... Types>
bool Cache<Policy, Types...>::IsDead(const Cache<Policy, Types...>::Element& element) const {
    if (!element.death_time_.has_value()) {
        return false;
    }

    return std::chrono::steady_clock::now() >= *element.death_time_;
}

template <Policies Policy, typename... Types>
void Cache<Policy, Types...>::ClearDead(const std::string& key) {
    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return;
    }

    if (!IsDead(iterator->second)) {
        return;
    }

    memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
    data_.erase(iterator);
}

template <Policies Policy, typename... Types>
void Cache<Policy, Types...>::ClearAllDead() {
    for (auto iterator = data_.begin(); iterator != data_.end();) {
        if (IsDead(iterator->second)) {
            memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
            iterator = data_.erase(iterator);
        } else {
            ++iterator;
        }
    }
}

template <Policies Policy, typename... Types>
typename Cache<Policy, Types...>::Variant* Cache<Policy, Types...>::Get(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);
    if (iterator == data_.end()) {
        return nullptr;
    }

    return &iterator->second.value_;
}

template <Policies Policy, typename... Types>
bool Cache<Policy, Types...>::IsContain(const std::string& key) {
    auto* value = Get(key);

    if (!value) {
        return false;
    }

    return true;
}

template <Policies Policy, typename... Types>
bool Cache<Policy, Types...>::Put(const std::string& key, Variant value) {
    ClearDead(key);

    std::size_t old_memory = 0;

    auto iterator = data_.find(key);
    if (iterator != data_.end()) {
        old_memory = KeyMemoryStored(key) + MemoryStored(iterator->second);
    }

    Element new_element{std::move(value), std::nullopt};
    std::size_t new_memory = KeyMemoryStored(key) + MemoryStored(new_element);

    if (!CanStore(old_memory, new_memory)) {
        return false;
    }

    memory_usage_ = memory_usage_ - old_memory + new_memory;
    data_[key] = std::move(new_element);

    return true;
}

template <Policies Policy, typename... Types>
bool Cache<Policy, Types...>::Remove(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return false;
    }

    memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
    data_.erase(iterator);

    return true;
}

template <Policies Policy, typename... Types>
void Cache<Policy, Types...>::ClearDB() {
    data_.clear();
    memory_usage_ = 0;
}

template <Policies Policy, typename... Types>
std::size_t Cache<Policy, Types...>::Size() {
    ClearAllDead();
    return data_.size();
}

template <Policies Policy, typename... Types>
bool Cache<Policy, Types...>::SetTTL(const std::string& key, int seconds) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return false;
    }

    if (seconds <= 0) {
        memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
        data_.erase(iterator);
        return true;
    }

    iterator->second.death_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);

    return true;
}

template <Policies Policy, typename... Types>
int64_t Cache<Policy, Types...>::GetTTL(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return -2;
    }

    if (!iterator->second.death_time_.has_value()) {
        return -1;
    }

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
        *iterator->second.death_time_ - std::chrono::steady_clock::now()
    );

    if (seconds.count() < 0) {
        memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
        data_.erase(iterator);
        return -2;
    }

    return seconds.count();
}

template <Policies Policy, typename... Types>
std::size_t Cache<Policy, Types...>::MemoryStored(const Variant& value) const {
    return std::visit([](const auto& element) {
        return ObjectMemory(element);
    }, value);
}

template <Policies Policy, typename... Types>
std::size_t Cache<Policy, Types...>::MemoryStored(const Element& element) const {
    return MemoryStored(element.value_);
}

template <Policies Policy, typename... Types>
std::size_t Cache<Policy, Types...>::MemoryStored(const std::string& key) {
    auto* value = Get(key);

    if (!value) {
        return 0;
    }

    return KeyMemoryStored(key) + MemoryStored(*value);
}

template <Policies Policy, typename... Types>
std::size_t Cache<Policy, Types...>::KeyMemoryStored(const std::string& key) const {
    return sizeof(std::string) + key.capacity();
}

template <Policies Policy, typename... Types>
bool Cache<Policy, Types...>::CanStore(std::size_t old_memory, std::size_t new_memory) const {
    if (max_memory_ == 0) {
        return true;
    }

    return memory_usage_ - old_memory + new_memory <= max_memory_;
}

template <Policies Policy, typename... Types>
void Cache<Policy, Types...>::SetMaxMemory(std::size_t memory_size) {
    max_memory_ = memory_size;
}

template <Policies Policy, typename... Types>
std::size_t Cache<Policy, Types...>::GetMaxMemory() const {
    return max_memory_;
}

template <Policies Policy, typename... Types>
std::uint64_t Cache<Policy, Types...>::GetEvictionCount() const {
    return 0;
}

template <Policies Policy, typename... Types>
std::size_t Cache<Policy, Types...>::EstimateMemoryBytes() const {
    std::size_t bytes = sizeof(*this);
    return bytes;
}