#include "commands.hpp"
#include "result_types.hpp"

#include <cctype>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

Result WrongType() {
    return ErrorResult("WRONGTYPE Operation against a key holding the wrong kind of value");
}

bool IsError(const Result& result) {
    return std::get_if<ErrorResult>(&result);
}

std::string Normalize(std::string key) {
    for (char& s: key) {
        s = static_cast<char>(std::toupper(static_cast<unsigned char>(s)));
    }
    return key;
}

bool CheckEqualArguments(const std::vector<std::string>& arguments, std::size_t count) {
    return arguments.size() == count;
}

bool CheckMinimalArguments(const std::vector<std::string>& arguments, std::size_t count) {
    return arguments.size() >= count;
}