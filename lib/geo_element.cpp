#include "geo_element.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

int GeoElement::Add(double longitude, double latitude, const std::string& member) {
    bool is_new = !value_.contains(member);

    value_[member] = GeoPoint{longitude, latitude};

    return is_new ? 1 : 0;
}

bool GeoElement::Contains(const std::string& member) const {
    return value_.contains(member);
}

std::optional<GeoPoint> GeoElement::GetPoint(const std::string& member) const {
    auto iterator = value_.find(member);

    if (iterator == value_.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

const std::unordered_map<std::string, GeoPoint>& GeoElement::GetAll() const {
    return value_;
}

std::size_t GeoElement::Size() const {
    return value_.size();
}

std::size_t GeoElement::MemoryStored() const {
    std::size_t ans = sizeof(GeoElement);

    for (const auto& [member, point]: value_) {
        ans += sizeof(std::string) + member.capacity();
        ans += sizeof(GeoPoint);
    }

    return ans;
}