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

#include "storage_types.hpp"
#include "string_element.hpp"
#include "list_element.hpp"
#include "set_element.hpp"
#include "geo_element.hpp"
#include "result_types.hpp"

#include "commands.hpp"

template <DataBaseStorage Storage = DefaultStorage>
class DataBase {
public:
    using Variant = DataBaseStoredTypes;
    using Element = DataBaseElement;
    using Command = Result (*)(DataBase<Storage>&, const std::vector<std::string>&);

    DataBase();

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
    Storage data_;
    std::unordered_map<std::string, Command> commands_;
    void RegisterCommand(const std::string& name, Command command);

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

#include "commands_string.tpp"
#include "commands_list.tpp"
#include "commands_set.tpp"
#include "commands_general.tpp"
#include "commands_geo.tpp"

#include "data_base.tpp"