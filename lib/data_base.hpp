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

#include "string_element.hpp"
#include "list_element.hpp"
#include "set_element.hpp"
#include "geo_element.hpp"
#include "result_types.hpp"

class DataBase {
public:
    using Variant = std::variant<StringElement, ListElement, SetElement, GeoElement>;
    using Command = Result (*)(DataBase&, const std::vector<std::string>&);
    using Death_Time = std::chrono::steady_clock::time_point;

    struct Element {
        Variant value_;
        std::optional<Death_Time> death_time_;
    };

    DataBase();

    void RegisterCommand(const std::string& name, Command command);

    Result Execute(const std::string& line);

    bool IsContain(const std::string& key);
    Variant* GetElement(const std::string& key);
    Result PutElement(const std::string& key, Variant value);
    Result UpdateElement(const std::string& key,
        const std::function<Result(Variant&)>& action);
    bool RemoveElement(const std::string& key);
    void ClearDB();
    std::size_t Size();

    bool SetTTL(const std::string& key, int seconds);
    int64_t GetTTL(const std::string& key);

    std::vector<std::string> GetKeys();

    std::size_t MemoryStored(const std::string& key);
    void SetMaxMemory(std::size_t max_memory);
    std::size_t GetMaxMemory() const;  

private:
    std::unordered_map<std::string, Element> data_;
    std::unordered_map<std::string, Command> commands_;

    bool IsDead(const Element& element) const;
    void ClearDead(const std::string& key);
    void ClearAllDead();

    std::size_t max_memory_ = 0;
    std::size_t memory_usage_ = 0;

    std::vector<std::string> ParseLine(const std::string& line);

    std::size_t MemoryStored(const Variant& value) const;
    std::size_t MemoryStored(const Element& element) const;
    std::size_t KeyMemoryStored(const std::string& key) const;
    bool CanStore(std::size_t old_memory, std::size_t new_memory) const; 
};