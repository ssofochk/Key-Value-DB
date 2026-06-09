#include "data_base.hpp"
#include "commands.hpp"
#include "result_types.hpp"

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

DataBase::DataBase() {
    RegisterCommand("SET", &Set);
    RegisterCommand("GET", &Get);
    RegisterCommand("STRLEN", &StrLen);
    RegisterCommand("APPEND", &Append);

    RegisterCommand("EXPIRE", &Expire);
    RegisterCommand("TTL", &TTL);

    RegisterCommand("LPUSH", &LPush);
    RegisterCommand("RPUSH", &RPush);
    RegisterCommand("LPOP", &LPop);
    RegisterCommand("RPOP", &RPop);
    RegisterCommand("LLEN", &LLen);
    RegisterCommand("LRANGE", &LRange);
    RegisterCommand("LINDEX", &LIndex);
    RegisterCommand("LSET", &LSet);
    RegisterCommand("LINSERT", &LInsert);

    RegisterCommand("SADD", &SAdd);
    RegisterCommand("SREM", &SRem);
    RegisterCommand("SISMEMBER", &SIsMember);
    RegisterCommand("SMEMBERS", &SMembers);
    RegisterCommand("SCARD", &SCard);
    RegisterCommand("SUNION", &SUnion);
    RegisterCommand("SINTER", &SInter);
    RegisterCommand("SDIFF", &SDiff);
    RegisterCommand("SMOVE", &SMove);

    RegisterCommand("TYPE", &Type);
    RegisterCommand("DEL", &Del);
    RegisterCommand("EXISTS", &Exists);
    RegisterCommand("KEYS", &Keys);
    RegisterCommand("FLUSHDB", &FlushDB);
    RegisterCommand("CONFIG", &Config);
    RegisterCommand("DBSIZE", &DBSize);
    RegisterCommand("MEMORY", &Memory);

    RegisterCommand("GEOADD", &GeoAdd);
    RegisterCommand("GEOPOS", &GeoPos);
    RegisterCommand("GEODIST", &GeoDist);
    RegisterCommand("GEOSEARCH", &GeoSearch);
    RegisterCommand("GEOSEARCHSTORE", &GeoSearchStore);
}

void DataBase::RegisterCommand(const std::string& name, Command command) {
    commands_[Normalize(name)] = command;
}

std::vector<std::string> DataBase::ParseLine(const std::string& line) {
    std::vector<std::string> arguments;
    std::string argument;

    bool in_quotes = false;

    for (char symbol: line) {
        if (symbol == '"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(symbol)) && !in_quotes) {
            if (!argument.empty()) {
                arguments.push_back(argument);
                argument.clear();
            }

            continue;
        }

        argument.push_back(symbol);
    }

    if (!argument.empty()) {
        arguments.push_back(argument);
    }

    return arguments;
}

Result DataBase::Execute(const std::string& line) {
    std::vector<std::string> arguments = ParseLine(line);

    if (arguments.empty()) {
        return ErrorResult("no command");
    }

    std::string command = Normalize(arguments[0]);

    auto iterator = commands_.find(command);

    if (iterator == commands_.end()) {
        return ErrorResult("invalid command");
    }

    return iterator->second(*this, arguments);
}

bool DataBase::IsDead(const Element& element) const {
    if (!element.death_time_.has_value()) {
        return false;
    }

    return std::chrono::steady_clock::now() >= *element.death_time_;
}

void DataBase::ClearDead(const std::string& key) {
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

void DataBase::ClearAllDead() {
    for (auto iterator = data_.begin(); iterator != data_.end();) {
        if (IsDead(iterator->second)) {
            memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
            iterator = data_.erase(iterator);
        } else {
            ++iterator;
        }
    }
}

DataBase::Variant* DataBase::GetElement(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);
    if (iterator == data_.end()) {
        return nullptr;
    }

    return &iterator->second.value_;
}

bool DataBase::IsContain(const std::string& key) {
    auto* value = GetElement(key);

    if (!value) {
        return false;
    }

    return true;
}

Result DataBase::PutElement(const std::string& key, Variant value) {
    ClearDead(key);

    std::size_t old_memory = 0;

    auto iterator = data_.find(key);
    if (iterator != data_.end()) {
        old_memory = KeyMemoryStored(key) + MemoryStored(iterator->second);
    }

    Element new_element{std::move(value), std::nullopt};
    std::size_t new_memory = KeyMemoryStored(key) + MemoryStored(new_element);

    if (!CanStore(old_memory, new_memory)) {
        return ErrorResult("OOM command not allowed when used memory > 'maxmemory'");
    }

    memory_usage_ = memory_usage_ - old_memory + new_memory;
    data_[key] = std::move(new_element);

    return OkResult{};
}

Result DataBase::UpdateElement(const std::string& key,
    const std::function<Result(Variant&)>& action) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return NilResult{};
    }

    Variant new_value = iterator->second.value_;

    Result result = action(new_value);

    if (IsError(result)) {
        return result;
    }

    std::size_t old_memory = KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
    std::size_t new_memory = KeyMemoryStored(iterator->first) + MemoryStored(new_value);

    if (!CanStore(old_memory, new_memory)) {
        return ErrorResult("OOM command not allowed when used memory > 'maxmemory'");
    }

    memory_usage_ = memory_usage_ - old_memory + new_memory;
    iterator->second.value_ = std::move(new_value);

    return result;
}

bool DataBase::RemoveElement(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return false;
    }

    memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
    data_.erase(iterator);

    return true;
}

void DataBase::ClearDB() {
    data_.clear();
    memory_usage_ = 0;
}

std::size_t DataBase::Size() {
    ClearAllDead();
    return data_.size();
}

std::vector<std::string> DataBase::GetKeys() {
    ClearAllDead();
    std::vector<std::string> ans;

    for (const auto& [key, value]: data_) {
        ans.push_back(key);
    }
    return ans;
}

bool DataBase::SetTTL(const std::string& key, int seconds) {
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

int64_t DataBase::GetTTL(const std::string& key) {
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
        return -2;
    } 

    return seconds.count();
}

std::size_t DataBase::MemoryStored(const Variant& value) const {
    return std::visit([](const auto& element) {
        return element.MemoryStored();
    }, value);
}

std::size_t DataBase::MemoryStored(const Element& element) const {
    return MemoryStored(element.value_);
}

std::size_t DataBase::MemoryStored(const std::string& key) {
    auto* value = GetElement(key);

    if (!value) {
        return 0;
    }

    return KeyMemoryStored(key) + MemoryStored(*value);
}

std::size_t DataBase::KeyMemoryStored(const std::string& key) const {
    return sizeof(std::string) + key.capacity();
}

bool DataBase::CanStore(std::size_t old_memory, std::size_t new_memory) const {
    if (max_memory_ == 0) {
        return true;
    }

    return memory_usage_ - old_memory + new_memory <= max_memory_;
}

void DataBase::SetMaxMemory(std::size_t memory_size) {
    max_memory_ = memory_size;
}

std::size_t DataBase::GetMaxMemory() const {
    return max_memory_;
}