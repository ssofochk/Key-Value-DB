#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "policies.hpp"

template <Policies Policy, typename... Types>
class Cache {
public:
    static_assert(sizeof...(Types) > 0);

    using Variant = std::variant<Types...>;
    using DeathTime = std::chrono::steady_clock::time_point;

    struct CacheElement {
        Variant value_;
        std::optional<DeathTime> death_time_;
    };

    using Element = CacheElement;

    bool IsContain(const std::string& key);
    Variant* Get(const std::string& key);
    bool Put(const std::string& key, Variant value);
    bool Remove(const std::string& key);
    void ClearDB();
    std::size_t Size();

    bool SetTTL(const std::string& key, int seconds);
    int64_t GetTTL(const std::string& key);

    std::size_t MemoryStored(const std::string& key);
    void SetMaxMemory(std::size_t max_memory);
    std::size_t GetMaxMemory() const;  

    std::uint64_t GetEvictionCount() const;

private:
    std::unordered_map<std::string, Element> data_;

    bool IsDead(const Element& element) const;
    void ClearDead(const std::string& key);
    void ClearAllDead();

    std::size_t max_memory_ = 0;
    std::size_t memory_usage_ = 0;

    std::size_t MemoryStored(const Variant& value) const;
    std::size_t MemoryStored(const Element& element) const;
    std::size_t KeyMemoryStored(const std::string& key) const;
    bool CanStore(std::size_t old_memory, std::size_t new_memory) const; 
};

#include "cache.tpp"