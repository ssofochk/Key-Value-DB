#include "commands.hpp"
#include "data_base.hpp"
#include "geo_element.hpp"
#include "list_element.hpp"
#include "result_types.hpp"
#include "set_element.hpp"
#include "string_element.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

Result Type(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& key = arguments[1];

    auto* value = data_base.GetElement(key);
    if (!value) {
        return StringResult("none");
    }

    if (std::get_if<StringElement>(value)) {
        return StringResult("string");
    }

    if (std::get_if<ListElement>(value)) {
        return StringResult("list");
    }

    if (std::get_if<SetElement>(value)) {
        return StringResult("set");
    }

    if (std::get_if<GeoElement>(value)) {
        return StringResult{"zset"};
    }

    return StringResult("none");
}

Result Del(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    int removed = 0;

    for (std::size_t i = 1; i < arguments.size(); ++i) {
        if (data_base.RemoveElement(arguments[i])) {
            ++removed;
        }
    }

    return IntegerResult(removed);
}

Result Exists(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    int exists = 0;

    for (std::size_t i = 1; i < arguments.size(); ++i) {
        if (data_base.IsContain(arguments[i])) {
            ++exists;
        }
    }

    return IntegerResult(exists);
}

bool TryMatch(const std::string& text, const std::string& pattern,
    std::size_t text_index, std::size_t pattern_index) {
    if (pattern_index == pattern.size()) {
        return text_index == text.size();
    }

    if (pattern[pattern_index] == '*') {
        for (std::size_t i = text_index; i <= text.size(); ++i) {
            if (TryMatch(text, pattern, i, pattern_index + 1)) {
                return true;
            }
        }

        return false;
    }

    if (text_index == text.size()) {
        return false;
    }

    if (pattern[pattern_index] == '?' || pattern[pattern_index] == text[text_index]) {
        return TryMatch(text, pattern, text_index + 1, pattern_index + 1);
    }

    return false;
}

bool IsMatch(const std::string& text, const std::string& pattern) {
    return TryMatch(text, pattern, 0, 0);
}

Result Keys(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    const std::string& pattern = arguments[1];
    ListResult result;
    auto keys = data_base.GetKeys();

    for (const auto& key: keys) {
        if (IsMatch(key, pattern)) {
            result.value_.push_back(key);
        }
    }
    return result;
}

Result FlushDB(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 1)) {
        return ErrorResult("wrong number of arguments");
    }

    data_base.ClearDB();
    return OkResult{};
}

Result DBSize(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 1)) {
        return ErrorResult("wrong number of arguments");
    }

    return IntegerResult(data_base.Size());
}

Result Config(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }
    std::string action = Normalize(arguments[1]);

    if (action == "GET") {
        if (!CheckEqualArguments(arguments, 3)) {
            return ErrorResult("wrong number of arguments");
        }

        std::string parameter = Normalize(arguments[2]);

        if (parameter != "MAXMEMORY") {
            return ListResult{};
        }

        ListResult result;
        result.value_.push_back("maxmemory");
        result.value_.push_back(std::to_string(data_base.GetMaxMemory()));
        return result;
    }

    if (action == "SET") {
        if (!CheckEqualArguments(arguments, 4)) {
            return ErrorResult("wrong number of arguments");
        }

        std::string parameter = Normalize(arguments[2]);

        if (parameter != "MAXMEMORY") {
            return ErrorResult("Unsupported CONFIG parameter");
        }

        std::size_t max_memory;

        try {
            max_memory = std::stoull(arguments[3]);
        } catch (const std::invalid_argument&) {
            return ErrorResult("value is not an integer or out of range");
        } catch (const std::out_of_range&) {
            return ErrorResult("value is not an integer or out of range");
        }

        data_base.SetMaxMemory(max_memory);

        return OkResult{};
    }

    return ErrorResult("syntax error");
}

Result Memory(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    std::string action = Normalize(arguments[1]);

    if (action != "USAGE") {
        return ErrorResult("syntax error");
    }

    if (!data_base.IsContain(arguments[2])) {
        return NilResult{};
    }

    return IntegerResult(data_base.MemoryStored(arguments[2]));
}

Result Expire(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 3)) {
        return ErrorResult("wrong number of arguments");
    }

    int seconds;

    try {
        seconds = std::stoi(arguments[2]);
    } catch (const std::invalid_argument&) {
        return ErrorResult("value is not an integer or out of range");
    } catch (const std::out_of_range&) {
        return ErrorResult("value is not an integer or out of range");
    }

    return IntegerResult(data_base.SetTTL(arguments[1], seconds) ? 1 : 0);
}

Result TTL(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckEqualArguments(arguments, 2)) {
        return ErrorResult("wrong number of arguments");
    }

    return IntegerResult(data_base.GetTTL(arguments[1]));
}