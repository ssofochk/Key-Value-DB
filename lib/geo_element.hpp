#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

struct GeoPoint {
    double longitude_;
    double latitude_;
};

struct GeoElement {
private:
    std::unordered_map<std::string, GeoPoint> value_;

public:
    int Add(double longitude, double latitude, const std::string& member);

    bool Contains(const std::string& member) const;
    std::optional<GeoPoint> GetPoint(const std::string& member) const;

    const std::unordered_map<std::string, GeoPoint>& GetAll() const;

    std::size_t Size() const;
    std::size_t MemoryStored() const;
};