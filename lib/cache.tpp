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

template <Policies Policy>
bool Cache<Policy>::IsDead(const Cache<Policy>::Element& element) const {
    if (!element.death_time_.has_value()) {
        return false;
    }

    return std::chrono::steady_clock::now() >= *element.death_time_;
}

template <Policies Policy>
void Cache<Policy>::ClearDead(const std::string& key) {
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

template <Policies Policy>
void Cache<Policy>::ClearAllDead() {
    for (auto iterator = data_.begin(); iterator != data_.end();) {
        if (IsDead(iterator->second)) {
            memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
            iterator = data_.erase(iterator);
        } else {
            ++iterator;
        }
    }
}

template <Policies Policy>
std::string* Cache<Policy>::Get(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);
    if (iterator == data_.end()) {
        return nullptr;
    }

    return &iterator->second.value_;
}

template <Policies Policy>
bool Cache<Policy>::IsContain(const std::string& key) {
    auto* value = Get(key);

    if (!value) {
        return false;
    }

    return true;
}

template <Policies Policy>
bool Cache<Policy>::Put(const std::string& key, std::string value) {
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

template <Policies Policy>
bool Cache<Policy>::Remove(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return false;
    }

    memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
    data_.erase(iterator);

    return true;
}

template <Policies Policy>
void Cache<Policy>::ClearDB() {
    data_.clear();
    memory_usage_ = 0;
}

template <Policies Policy>
std::size_t Cache<Policy>::Size() {
    ClearAllDead();
    return data_.size();
}

template <Policies Policy>
bool Cache<Policy>::SetTTL(const std::string& key, int seconds) {
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

template <Policies Policy>
int64_t Cache<Policy>::GetTTL(const std::string& key) {
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

template <Policies Policy>
std::size_t Cache<Policy>::MemoryStored(const std::string& value) const {
    return sizeof(std::string) + value.capacity();
}

template <Policies Policy>
std::size_t Cache<Policy>::MemoryStored(const Element& element) const {
    return MemoryStored(element.value_);
}

template <Policies Policy>
std::size_t Cache<Policy>::MemoryStored(const std::string& key) {
    auto* value = Get(key);

    if (!value) {
        return 0;
    }

    return KeyMemoryStored(key) + MemoryStored(*value);
}

template <Policies Policy>
std::size_t Cache<Policy>::KeyMemoryStored(const std::string& key) const {
    return sizeof(std::string) + key.capacity();
}

template <Policies Policy>
bool Cache<Policy>::CanStore(std::size_t old_memory, std::size_t new_memory) const {
    if (max_memory_ == 0) {
        return true;
    }

    return memory_usage_ - old_memory + new_memory <= max_memory_;
}

template <Policies Policy>
void Cache<Policy>::SetMaxMemory(std::size_t memory_size) {
    max_memory_ = memory_size;
}

template <Policies Policy>
std::size_t Cache<Policy>::GetMaxMemory() const {
    return max_memory_;
}

template <Policies Policy>
std::uint64_t Cache<Policy>::GetEvictionCount() const {
    return 0;
}

template <Policies Policy>
std::size_t Cache<Policy>::EstimateMemoryBytes() const {
    std::size_t bytes = sizeof(*this);
    return bytes;
}

template <Policies Policy>
uint64_t Cache<Policy>::GetHits() const {
    return 1;
}

template <Policies Policy>
uint64_t Cache<Policy>::GetMisses() const {
    return 1;
}
