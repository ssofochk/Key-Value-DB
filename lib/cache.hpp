#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "policies.hpp"

template <Policies Policy>
struct CachePolicyMetadata {};

template <>
struct CachePolicyMetadata<Policies::LRU> {
    std::uint64_t last_access_ = 0;
};

template <>
struct CachePolicyMetadata<Policies::LFU> {
    std::uint64_t frequency_ = 0;
    std::uint64_t last_access_ = 0;
};

template <>
struct CachePolicyMetadata<Policies::RANDOM> {};

template <>
struct CachePolicyMetadata<Policies::NOEVICTION> {};

template <Policies Policy>
class Cache {
public:
    using DeathTime = std::chrono::steady_clock::time_point;
    using Metadata = CachePolicyMetadata<Policy>;

    struct CacheElement {
        std::string value_;
        std::optional<DeathTime> death_time_;
    };

    using Element = CacheElement;

    bool IsContain(const std::string& key);
    std::string* Get(const std::string& key);
    bool Put(const std::string& key, std::string value);
    bool Remove(const std::string& key);
    void ClearDB();
    std::size_t Size();

    bool SetTTL(const std::string& key, int seconds);
    int64_t GetTTL(const std::string& key);

    std::size_t MemoryStored(const std::string& key);
    void SetMaxMemory(std::size_t max_memory);
    std::size_t GetMaxMemory() const;  

    std::uint64_t GetEvictionCount() const;
    std::size_t EstimateMemoryBytes() const;

    uint64_t GetHits() const;
    uint64_t GetMisses() const;

private:
    std::unordered_map<std::string, Element> data_;
    std::unordered_map<std::string, Metadata> metadata_;

    bool IsDead(const Element& element) const;
    void ClearDead(const std::string& key);
    void ClearAllDead();

    std::size_t max_memory_ = 0;
    std::size_t memory_usage_ = 0;

    std::uint64_t misses_ = 0;
    std::uint64_t hits_ = 0;
    std::uint64_t eviction_count_ = 0;
    std::uint64_t access_clock_ = 0;

    std::size_t MemoryStored(const std::string& value) const;
    std::size_t MemoryStored(const Element& element) const;
    std::size_t KeyMemoryStored(const std::string& key) const;
    bool FitMemory(std::size_t old_memory, std::size_t new_memory);
    bool TryEvictByPolicy();
    void Touch(const std::string& key);
};

#include "cache.tpp"