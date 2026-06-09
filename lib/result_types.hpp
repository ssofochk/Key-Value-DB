#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

struct NilResult {};
struct OkResult {};
struct EmptyResult {};

struct IntegerResult {
    int64_t value_;
};

struct ListResult {
    std::vector<std::string> value_;
};

struct StringResult {
    std::string value_;
};

struct ErrorResult {
    std::string message_;
};

struct GeoResult {
    double longitude_;
    double latitude_;
};

struct GeoListResult {
    std::vector<std::variant<GeoResult, NilResult>> value_;
};

using Result = std::variant
    <StringResult,
    ListResult,
    IntegerResult,
    NilResult,
    OkResult,
    ErrorResult,
    EmptyResult,
    GeoResult,
    GeoListResult>;

