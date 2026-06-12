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

template <typename T>
std::size_t ObjectMemory(const T& value) {
    if constexpr (requires { value.capacity(); }) {
        return sizeof(T) + value.capacity();
    } else {
        return sizeof(T);
    }
}

template <typename... Types>
bool Cache<Types...>::IsDead(const Cache<Types...>::Element& element) const {
    if (!element.death_time_.has_value()) {
        return false;
    }

    return std::chrono::steady_clock::now() >= *element.death_time_;
}

template <typename... Types>
void Cache<Types...>::ClearDead(const std::string& key) {
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

template <typename... Types>
void Cache<Types...>::ClearAllDead() {
    for (auto iterator = data_.begin(); iterator != data_.end();) {
        if (IsDead(iterator->second)) {
            memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
            iterator = data_.erase(iterator);
        } else {
            ++iterator;
        }
    }
}

template <typename... Types>
typename Cache<Types...>::Variant* Cache<Types...>::Get(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);
    if (iterator == data_.end()) {
        return nullptr;
    }

    return &iterator->second.value_;
}

template <typename... Types>
bool Cache<Types...>::IsContain(const std::string& key) {
    auto* value = Get(key);

    if (!value) {
        return false;
    }

    return true;
}

template <typename... Types>
bool Cache<Types...>::Put(const std::string& key, Variant value) {
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

template <typename... Types>
bool Cache<Types...>::Remove(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return false;
    }

    memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
    data_.erase(iterator);

    return true;
}

template <typename... Types>
void Cache<Types...>::ClearDB() {
    data_.clear();
    memory_usage_ = 0;
}

template <typename... Types>
std::size_t Cache<Types...>::Size() {
    ClearAllDead();
    return data_.size();
}

template <typename... Types>
bool Cache<Types...>::SetTTL(const std::string& key, int seconds) {
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

template <typename... Types>
int64_t Cache<Types...>::GetTTL(const std::string& key) {
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

template <typename... Types>
std::size_t Cache<Types...>::MemoryStored(const Variant& value) const {
    return std::visit([](const auto& element) {
        return ObjectMemory(element);
    }, value);
}

template <typename... Types>
std::size_t Cache<Types...>::MemoryStored(const Element& element) const {
    return MemoryStored(element.value_);
}

template <typename... Types>
std::size_t Cache<Types...>::MemoryStored(const std::string& key) {
    auto* value = Get(key);

    if (!value) {
        return 0;
    }

    return KeyMemoryStored(key) + MemoryStored(*value);
}

template <typename... Types>
std::size_t Cache<Types...>::KeyMemoryStored(const std::string& key) const {
    return sizeof(std::string) + key.capacity();
}

template <typename... Types>
bool Cache<Types...>::CanStore(std::size_t old_memory, std::size_t new_memory) const {
    if (max_memory_ == 0) {
        return true;
    }

    return memory_usage_ - old_memory + new_memory <= max_memory_;
}

template <typename... Types>
void Cache<Types...>::SetMaxMemory(std::size_t memory_size) {
    max_memory_ = memory_size;
}

template <typename... Types>
std::size_t Cache<Types...>::GetMaxMemory() const {
    return max_memory_;
}