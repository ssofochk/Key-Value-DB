#include "data_base.hpp"
#include "result_types.hpp"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

bool IsExit(const std::string& line) {
    if (line.size() < 4) {
        return false;
    }

    bool is_exit = std::toupper(static_cast<unsigned char>(line[0])) == 'E' &&
        std::toupper(static_cast<unsigned char>(line[1])) == 'X' &&
        std::toupper(static_cast<unsigned char>(line[2])) == 'I' &&
        std::toupper(static_cast<unsigned char>(line[3])) == 'T';

    if (!is_exit) {
        return false;
    }

    return line.size() == 4 || std::isspace(static_cast<unsigned char>(line[4]));
}

bool ParseMemoryLimit(const std::string& value, std::size_t& result) {
    std::string number;
    std::string suffix;

    for (char symbol: value) {
        if (std::isdigit(static_cast<unsigned char>(symbol))) {
            if (!suffix.empty()) {
                return false;
            }

            number += symbol;
        } else {
            suffix += static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
        }
    }

    if (number.empty()) {
        return false;
    }

    std::size_t multiplier = 1;

    if (suffix.empty() || suffix == "b") {
        multiplier = 1;
    } else if (suffix == "kb") {
        multiplier = 1024;
    } else if (suffix == "mb") {
        multiplier = 1024 * 1024;
    } else if (suffix == "gb") {
        multiplier = 1024 * 1024 * 1024;
    } else {
        return false;
    }

    try {
        result = std::stoull(number) * multiplier;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }

    return true;
}

void PrintString(const std::string& value) {
    std::cout << '"' << value << '"' << '\n';
}

void PrintValue(const StringResult& value) {
    PrintString(value.value_);
}

void PrintValue(const IntegerResult& value) {
    std::cout << "(integer) " << value.value_ << '\n';
}

void PrintValue(const ErrorResult& value) {
    std::cerr << "(error) " << value.message_ << '\n';
}

void PrintValue(const OkResult& value) {
    std::cout << "OK" << '\n';
}

void PrintValue(const NilResult& value) {
    std::cout << "(nil)" << '\n';
}

void PrintValue(const EmptyResult& value) {
    std::cout << "(empty array)" << '\n';
}

void PrintValue(const ListResult& value) {
    if (value.value_.empty()) {
        std::cout << "(empty array)" << '\n';
        return;
    }

    for (std::size_t i = 0; i < value.value_.size(); ++i) {
        std::cout << i + 1 << ") ";
        PrintString(value.value_[i]);
    }
}

void PrintValue(const GeoResult& value) {
    std::cout << "1) ";
    PrintString(std::to_string(value.longitude_));

    std::cout << "2) ";
    PrintString(std::to_string(value.latitude_));
}

void PrintValue(const GeoListResult& value) {
    if (value.value_.empty()) {
        std::cout << "(empty array)" << '\n';
        return;
    }

    for (std::size_t i = 0; i < value.value_.size(); ++i) {
        std::cout << i + 1 << ") ";

        const auto& position = value.value_[i];

        if (std::get_if<NilResult>(&position)) {
            std::cout << "(nil)" << '\n';
        } else if (auto* geo = std::get_if<GeoResult>(&position)) {
            std::cout << "1) ";
            PrintString(std::to_string(geo->longitude_));

            std::cout << "   2) ";
            PrintString(std::to_string(geo->latitude_));
        }
    }
}

void PrintResult(const Result& result) {
    std::visit([](const auto& value) {
        PrintValue(value);
    }, result);
}

int main(int argc, char** argv) {
    DataBase data_base;

    if (argc == 3 && std::string(argv[1]) == "--maxmemory") {
        std::size_t max_memory;

        if (!ParseMemoryLimit(argv[2], max_memory)) {
            std::cerr << "(error) invalid maxmemory value" << '\n';
            return 1;
        }

        data_base.SetMaxMemory(max_memory);
    } else if (argc != 1) {
        std::cerr << "(error) invalid arguments" << '\n';
        return 1;
    }

    std::string line;

    while (std::getline(std::cin, line)) {
        if (IsExit(line)) {
            break;
        }

        Result result = data_base.Execute(line);
        PrintResult(result);
    }

    return 0;
}