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

template <class Storage>
DataBase<Storage>::DataBase() {
    RegisterCommand("SET", &Set<Storage>);
    RegisterCommand("GET", &Get<Storage>);
    RegisterCommand("STRLEN", &StrLen<Storage>);
    RegisterCommand("APPEND", &Append<Storage>);

    RegisterCommand("EXPIRE", &Expire<Storage>);
    RegisterCommand("TTL", &TTL<Storage>);

    RegisterCommand("LPUSH", &LPush<Storage>);
    RegisterCommand("RPUSH", &RPush<Storage>);
    RegisterCommand("LPOP", &LPop<Storage>);
    RegisterCommand("RPOP", &RPop<Storage>);
    RegisterCommand("LLEN", &LLen<Storage>);
    RegisterCommand("LRANGE", &LRange<Storage>);
    RegisterCommand("LINDEX", &LIndex<Storage>);
    RegisterCommand("LSET", &LSet<Storage>);
    RegisterCommand("LINSERT", &LInsert<Storage>);

    RegisterCommand("SADD", &SAdd<Storage>);
    RegisterCommand("SREM", &SRem<Storage>);
    RegisterCommand("SISMEMBER", &SIsMember<Storage>);
    RegisterCommand("SMEMBERS", &SMembers<Storage>);
    RegisterCommand("SCARD", &SCard<Storage>);
    RegisterCommand("SUNION", &SUnion<Storage>);
    RegisterCommand("SINTER", &SInter<Storage>);
    RegisterCommand("SDIFF", &SDiff<Storage>);
    RegisterCommand("SMOVE", &SMove<Storage>);

    RegisterCommand("TYPE", &Type<Storage>);
    RegisterCommand("DEL", &Del<Storage>);
    RegisterCommand("EXISTS", &Exists<Storage>);
    RegisterCommand("KEYS", &Keys<Storage>);
    RegisterCommand("FLUSHDB", &FlushDB<Storage>);
    RegisterCommand("CONFIG", &Config<Storage>);
    RegisterCommand("DBSIZE", &DBSize<Storage>);
    RegisterCommand("MEMORY", &Memory<Storage>);

    RegisterCommand("GEOADD", &GeoAdd<Storage>);
    RegisterCommand("GEOPOS", &GeoPos<Storage>);
    RegisterCommand("GEODIST", &GeoDist<Storage>);
    RegisterCommand("GEOSEARCH", &GeoSearch<Storage>);
    RegisterCommand("GEOSEARCHSTORE", &GeoSearchStore<Storage>);
}

template <class Storage>
void DataBase<Storage>::RegisterCommand(const std::string& name, Command command) {
    commands_[Normalize(name)] = command;
}

template <class Storage>
std::vector<std::string> DataBase<Storage>::ParseLine(const std::string& line) {
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

template <class Storage>
Result DataBase<Storage>::Execute(const std::string& line) {
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

template <class Storage>
bool DataBase<Storage>::IsDead(const Element& element) const {
    if (!element.death_time_.has_value()) {
        return false;
    }

    return std::chrono::steady_clock::now() >= *element.death_time_;
}

template <class Storage>
void DataBase<Storage>::ClearDead(const std::string& key) {
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

template <class Storage>
void DataBase<Storage>::ClearAllDead() {
    for (auto iterator = data_.begin(); iterator != data_.end();) {
        if (IsDead(iterator->second)) {
            memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
            iterator = data_.erase(iterator);
        } else {
            ++iterator;
        }
    }
}

template <class Storage>
typename DataBase<Storage>::Variant* DataBase<Storage>::GetElement(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);
    if (iterator == data_.end()) {
        return nullptr;
    }

    return &iterator->second.value_;
}

template <class Storage>
bool DataBase<Storage>::IsContain(const std::string& key) {
    auto* value = GetElement(key);

    if (!value) {
        return false;
    }

    return true;
}

template <class Storage>
Result DataBase<Storage>::PutElement(const std::string& key, Variant value) {
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

template <class Storage>
Result DataBase<Storage>::UpdateElement(const std::string& key,
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

template <class Storage>
bool DataBase<Storage>::RemoveElement(const std::string& key) {
    ClearDead(key);

    auto iterator = data_.find(key);

    if (iterator == data_.end()) {
        return false;
    }

    memory_usage_ -= KeyMemoryStored(iterator->first) + MemoryStored(iterator->second);
    data_.erase(iterator);

    return true;
}

template <class Storage>
void DataBase<Storage>::ClearDB() {
    data_.clear();
    memory_usage_ = 0;
}

template <class Storage>
std::size_t DataBase<Storage>::Size() {
    ClearAllDead();
    return data_.size();
}

template <class Storage>
std::vector<std::string> DataBase<Storage>::GetKeys() {
    ClearAllDead();
    std::vector<std::string> ans;

    for (const auto& [key, value]: data_) {
        ans.push_back(key);
    }
    return ans;
}

template <class Storage>
bool DataBase<Storage>::SetTTL(const std::string& key, int seconds) {
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

template <class Storage>
int64_t DataBase<Storage>::GetTTL(const std::string& key) {
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

template <class Storage>
std::size_t DataBase<Storage>::MemoryStored(const Variant& value) const {
    return std::visit([](const auto& element) {
        return element.MemoryStored();
    }, value);
}

template <class Storage>
std::size_t DataBase<Storage>::MemoryStored(const Element& element) const {
    return MemoryStored(element.value_);
}

template <class Storage>
std::size_t DataBase<Storage>::MemoryStored(const std::string& key) {
    auto* value = GetElement(key);

    if (!value) {
        return 0;
    }

    return KeyMemoryStored(key) + MemoryStored(*value);
}

template <class Storage>
std::size_t DataBase<Storage>::KeyMemoryStored(const std::string& key) const {
    return sizeof(std::string) + key.capacity();
}

template <class Storage>
bool DataBase<Storage>::CanStore(std::size_t old_memory, std::size_t new_memory) const {
    if (max_memory_ == 0) {
        return true;
    }

    return memory_usage_ - old_memory + new_memory <= max_memory_;
}

template <class Storage>
void DataBase<Storage>::SetMaxMemory(std::size_t memory_size) {
    max_memory_ = memory_size;
}

template <class Storage>
std::size_t DataBase<Storage>::GetMaxMemory() const {
    return max_memory_;
}