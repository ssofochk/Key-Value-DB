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
#include <random>

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
    metadata_.erase(iterator->first);
    data_.erase(iterator);
}

template <Policies Policy>
void Cache<Policy>::ClearAllDead() {
    for (auto iterator = data_.begin(); iterator != data_.end();) {
        if (IsDead(iterator->second)) {
            memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
            metadata_.erase(iterator->first);
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
        ++misses_;
        return nullptr;
    }

    ++hits_;
    Touch(key);

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
    std::optional<DeathTime> old_death_time = std::nullopt;

    auto iterator = data_.find(key);
    if (iterator != data_.end()) {
        old_memory = KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
        old_death_time = iterator->second.death_time_;
    }

    Element new_element{std::move(value), old_death_time};
    std::size_t new_memory = KeyMemoryStored(key) + MemoryStored(new_element);

    if (!FitMemory(old_memory, new_memory)) {
        return false;
    }

    memory_usage_ = memory_usage_ - old_memory + new_memory;
    data_[key] = std::move(new_element);

    metadata_.try_emplace(key);
    Touch(key);

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
    metadata_.erase(iterator->first);
    data_.erase(iterator);

    return true;
}

template <Policies Policy>
void Cache<Policy>::ClearDB() {
    data_.clear();
    metadata_.clear();
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
        metadata_.erase(iterator->first);
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
        metadata_.erase(iterator->first);
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
    return MemoryStored(element.value_) + sizeof(element.death_time_);
}

template <Policies Policy>
std::size_t Cache<Policy>::MemoryStored(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);
    if (iterator == data_.end()) {
        return 0;
    }

    return KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
}

template <Policies Policy>
std::size_t Cache<Policy>::KeyMemoryStored(const std::string& key) const {
    return sizeof(std::string) + key.capacity();
}

template <Policies Policy>
void Cache<Policy>::SetMaxMemory(std::size_t memory_size) {
    max_memory_ = memory_size;
    ClearAllDead();

    if (max_memory_ == 0) {
        return;
    }

    while (memory_usage_ > max_memory_) {
        if (!TryEvictByPolicy()) {
            break;
        }
    }
}

template <Policies Policy>
std::size_t Cache<Policy>::GetMaxMemory() const {
    return max_memory_;
}

template <Policies Policy>
std::uint64_t Cache<Policy>::GetEvictionCount() const {
    return eviction_count_;
}

template <Policies Policy>
std::size_t Cache<Policy>::EstimateMemoryBytes() const {
    return sizeof(*this) +
        memory_usage_ +
        data_.bucket_count() * sizeof(void*) +
        metadata_.bucket_count() * sizeof(void*) +
        metadata_.size() * sizeof(Metadata);
}

template <Policies Policy>
uint64_t Cache<Policy>::GetHits() const {
    return hits_;
}

template <Policies Policy>
uint64_t Cache<Policy>::GetMisses() const {
    return misses_;
}

template <Policies Policy>
void Cache<Policy>::Touch(const std::string& key) {
    if constexpr (Policy == Policies::LRU) {
        metadata_[key].last_access_ = ++access_clock_;
    } else if constexpr (Policy == Policies::LFU) {
        auto& metadata = metadata_[key];
        ++metadata.frequency_;
        metadata.last_access_ = ++access_clock_;
    } else {
        return;
    }
}

template <Policies Policy>
bool Cache<Policy>::FitMemory(std::size_t old_memory, std::size_t new_memory) {
    if (max_memory_ == 0) {
        return true;
    }

    if (new_memory > max_memory_) {
        return false;
    }

    while (memory_usage_ - old_memory + new_memory > max_memory_) {
        if (!TryEvictByPolicy()) {
            return false;
        }
    }

    return true;
}

inline std::mt19937& Rand() {
    static std::mt19937 generator{std::random_device{}()};
    return generator;
}

template <Policies Policy>
bool Cache<Policy>::TryEvictByPolicy() {
    if constexpr (Policy == Policies::NOEVICTION) {
        return false;
    }

    if (data_.empty()) {
        return false;
    }

    auto victim = data_.begin();

    if constexpr (Policy == Policies::LRU) {
        for (auto iterator = data_.begin(); iterator != data_.end(); ++iterator) {
            const auto& current_metadata = metadata_.at(iterator->first);
            const auto& victim_metadata = metadata_.at(victim->first);

            if (current_metadata.last_access_ < victim_metadata.last_access_) {
                victim = iterator;
            }
        }
    } else if constexpr (Policy == Policies::LFU) {
        for (auto iterator = data_.begin(); iterator != data_.end(); ++iterator) {
            const auto& current_metadata = metadata_.at(iterator->first);
            const auto& victim_metadata = metadata_.at(victim->first);

            const bool lower_frequency =
                current_metadata.frequency_ < victim_metadata.frequency_;

            const bool same_frequency_older =
                current_metadata.frequency_ == victim_metadata.frequency_ &&
                current_metadata.last_access_ < victim_metadata.last_access_;

            if (lower_frequency || same_frequency_older) {
                victim = iterator;
            }
        }
    } else if constexpr (Policy == Policies::RANDOM) {
        std::uniform_int_distribution<std::size_t> distribution(0, data_.size() - 1);

        std::advance (
            victim,
            static_cast<std::ptrdiff_t>(distribution(Rand())));
    }

    memory_usage_ -= KeyMemoryStored(victim->first) + MemoryStored(victim->second);

    metadata_.erase(victim->first);
    data_.erase(victim);

    ++eviction_count_;

    return true;
}
