#include "commands.hpp"
#include "data_base.hpp"
#include "geo_element.hpp"
#include "result_types.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

const double kEarthRadiusMeters = 6372800.0;
const double kMinLongitude = -180.0;
const double kMaxLongitude = 180.0;
const double kMinLatitude = -85.05112878;
const double kMaxLatitude = 85.05112878;

struct GeoSearchRequest {
    GeoPoint center_;
    double radius_;
    bool ascending_ = false;
    bool descending_ = false;
    bool has_count_ = false;
    std::size_t count_ = 0;
};

struct GeoSearchValue {
    std::string member_;
    GeoPoint point_;
    double distance_;
};

double ToRadians(double degrees) {
    return degrees * std::numbers::pi / 180.0;
}

bool ParseDouble(const std::string& value, double& result) {
    try {
        std::size_t position = 0;
        result = std::stod(value, &position);

        if (position != value.size()) {
            return false;
        }
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }

    return true;
}

bool ParseSize(const std::string& value, std::size_t& result) {
    try {
        std::size_t position = 0;
        result = std::stoull(value, &position);

        if (position != value.size()) {
            return false;
        }
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }

    return true;
}

bool IsValidPosition(double longitude, double latitude) {
    return longitude >= kMinLongitude && longitude <= kMaxLongitude &&
        latitude >= kMinLatitude && latitude <= kMaxLatitude;
}

double DistanceMeters(const GeoPoint& first, const GeoPoint& second) {
    double first_latitude = ToRadians(first.latitude_);
    double second_latitude = ToRadians(second.latitude_);

    double latitude_delta = ToRadians(second.latitude_ - first.latitude_);
    double longitude_delta = ToRadians(second.longitude_ - first.longitude_);

    double sin_latitude = std::sin(latitude_delta / 2);
    double sin_longitude = std::sin(longitude_delta / 2);

    double value = sin_latitude * sin_latitude +
        std::cos(first_latitude) * std::cos(second_latitude) *
        sin_longitude * sin_longitude;

    double angle = 2 * std::atan2(std::sqrt(value), std::sqrt(1 - value));

    return kEarthRadiusMeters * angle;
}

bool GetUnitMultiplier(const std::string& unit, double& multiplier) {
    std::string normalized = Normalize(unit);

    if (normalized == "M") {
        multiplier = 1.0;
        return true;
    }

    if (normalized == "KM") {
        multiplier = 1000.0;
        return true;
    }

    if (normalized == "FT") {
        multiplier = 0.3048;
        return true;
    }

    if (normalized == "MI") {
        multiplier = 1609.344;
        return true;
    }

    return false;
}

std::string FormatDouble(double value) {
    std::string result = std::to_string(value);

    while (!result.empty() && result.back() == '0') {
        result.pop_back();
    }

    if (!result.empty() && result.back() == '.') {
        result.pop_back();
    }

    return result;
}

Result ParseGeoSearchRequest(const GeoElement& geo,
    const std::vector<std::string>& arguments, std::size_t start_index,
    GeoSearchRequest& request) {
    if (start_index >= arguments.size()) {
        return ErrorResult{"syntax error"};
    }

    std::size_t index = start_index;
    std::string from = Normalize(arguments[index]);

    if (from == "FROMMEMBER") {
        if (index + 1 >= arguments.size()) {
            return ErrorResult{"wrong number of arguments"};
        }

        auto point = geo.GetPoint(arguments[index + 1]);

        if (!point.has_value()) {
            return NilResult{};
        }

        request.center_ = *point;
        index += 2;
    } else if (from == "FROMLONLAT") {
        if (index + 2 >= arguments.size()) {
            return ErrorResult{"wrong number of arguments"};
        }

        double longitude;
        double latitude;

        if (!ParseDouble(arguments[index + 1], longitude) ||
            !ParseDouble(arguments[index + 2], latitude)) {
            return ErrorResult{"value is not a valid float"};
        }

        if (!IsValidPosition(longitude, latitude)) {
            return ErrorResult{"invalid longitude,latitude pair"};
        }

        request.center_ = GeoPoint{longitude, latitude};
        index += 3;
    } else {
        return ErrorResult{"syntax error"};
    }

    if (index + 2 >= arguments.size()) {
        return ErrorResult{"wrong number of arguments"};
    }

    if (Normalize(arguments[index]) != "BYRADIUS") {
        return ErrorResult{"syntax error"};
    }

    double radius;
    double multiplier;

    if (!ParseDouble(arguments[index + 1], radius)) {
        return ErrorResult{"value is not a valid float"};
    }

    if (!GetUnitMultiplier(arguments[index + 2], multiplier)) {
        return ErrorResult{"unsupported unit provided. please use M, KM, FT, MI"};
    }

    request.radius_ = radius * multiplier;
    index += 3;

    while (index < arguments.size()) {
        std::string option = Normalize(arguments[index]);

        if (option == "ASC") {
            if (request.descending_) {
                return ErrorResult{"syntax error"};
            }

            request.ascending_ = true;
            ++index;
        } else if (option == "DESC") {
            if (request.ascending_) {
                return ErrorResult{"syntax error"};
            }

            request.descending_ = true;
            ++index;
        } else if (option == "COUNT") {
            if (index + 1 >= arguments.size()) {
                return ErrorResult{"wrong number of arguments"};
            }

            std::size_t count;

            if (!ParseSize(arguments[index + 1], count)) {
                return ErrorResult{"value is not an integer or out of range"};
            }

            request.has_count_ = true;
            request.count_ = count;
            index += 2;
        } else {
            return ErrorResult{"syntax error"};
        }
    }

    return OkResult{};
}

std::vector<GeoSearchValue> FindByRadius(const GeoElement& geo,
    const GeoSearchRequest& request) {
    std::vector<GeoSearchValue> result;

    for (const auto& [member, point]: geo.GetAll()) {
        double distance = DistanceMeters(request.center_, point);

        if (distance <= request.radius_) {
            result.push_back(GeoSearchValue{member, point, distance});
        }
    }

    return result;
}

void PrepareGeoSearchResult(std::vector<GeoSearchValue>& result,
    const GeoSearchRequest& request) {
    if (request.ascending_) {
        std::sort(result.begin(), result.end(),
            [](const GeoSearchValue& lhs, const GeoSearchValue& rhs) {
                return lhs.distance_ < rhs.distance_;
            });
    }

    if (request.descending_) {
        std::sort(result.begin(), result.end(),
            [](const GeoSearchValue& lhs, const GeoSearchValue& rhs) {
                return lhs.distance_ > rhs.distance_;
            });
    }

    if (request.has_count_ && result.size() > request.count_) {
        result.resize(request.count_);
    }
}

Result GeoAdd(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (arguments.size() < 5 || arguments.size() % 3 != 2) {
        return ErrorResult{"wrong number of arguments"};
    }

    const std::string& key = arguments[1];

    std::vector<double> longitudes;
    std::vector<double> latitudes;
    std::vector<std::string> members;

    for (std::size_t i = 2; i < arguments.size(); i += 3) {
        double longitude;
        double latitude;

        if (!ParseDouble(arguments[i], longitude) || !ParseDouble(arguments[i + 1], latitude)) {
            return ErrorResult{"value is not a valid float"};
        }

        if (!IsValidPosition(longitude, latitude)) {
            return ErrorResult{"invalid longitude,latitude pair"};
        }

        longitudes.push_back(longitude);
        latitudes.push_back(latitude);
        members.push_back(arguments[i + 2]);
    }

    if (!data_base.IsContain(key)) {
        GeoElement geo;
        int added = 0;

        for (std::size_t i = 0; i < members.size(); ++i) {
            added += geo.Add(longitudes[i], latitudes[i], members[i]);
        }

        Result result = data_base.PutElement(key, std::move(geo));

        if (IsError(result)) {
            return result;
        }

        return IntegerResult{added};
    }

    return data_base.UpdateElement(key, [&](DataBase::Variant& value) -> Result {
        auto* geo = std::get_if<GeoElement>(&value);

        if (!geo) {
            return WrongType();
        }

        int added = 0;

        for (std::size_t i = 0; i < members.size(); ++i) {
            added += geo->Add(longitudes[i], latitudes[i], members[i]);
        }

        return IntegerResult{added};
    });
}

Result GeoPos(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 3)) {
        return ErrorResult{"wrong number of arguments"};
    }

    GeoListResult result;

    auto* value = data_base.GetElement(arguments[1]);

    if (!value) {
        for (std::size_t i = 2; i < arguments.size(); ++i) {
            result.value_.push_back(NilResult{});
        }

        return result;
    }

    auto* geo = std::get_if<GeoElement>(value);

    if (!geo) {
        return WrongType();
    }

    for (std::size_t i = 2; i < arguments.size(); ++i) {
        auto point = geo->GetPoint(arguments[i]);

        if (!point.has_value()) {
            result.value_.push_back(NilResult{});
        } else {
            result.value_.push_back(GeoResult{point->longitude_, point->latitude_});
        }
    }

    return result;
}

Result GeoDist(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!(CheckEqualArguments(arguments, 4) || CheckEqualArguments(arguments, 5))) {
        return ErrorResult{"wrong number of arguments"};
    }

    double multiplier = 1.0;

    if (arguments.size() == 5 && !GetUnitMultiplier(arguments[4], multiplier)) {
        return ErrorResult{"unsupported unit provided. please use M, KM, FT, MI"};
    }

    auto* value = data_base.GetElement(arguments[1]);

    if (!value) {
        return NilResult{};
    }

    auto* geo = std::get_if<GeoElement>(value);

    if (!geo) {
        return WrongType();
    }

    auto first = geo->GetPoint(arguments[2]);
    auto second = geo->GetPoint(arguments[3]);

    if (!first.has_value() || !second.has_value()) {
        return NilResult{};
    }

    double distance = DistanceMeters(*first, *second) / multiplier;

    return StringResult{FormatDouble(distance)};
}

Result GeoSearch(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 6)) {
        return ErrorResult{"wrong number of arguments"};
    }

    auto* value = data_base.GetElement(arguments[1]);

    if (!value) {
        return ListResult{};
    }

    auto* geo = std::get_if<GeoElement>(value);

    if (!geo) {
        return WrongType();
    }

    GeoSearchRequest request;

    Result parse_result = ParseGeoSearchRequest(*geo, arguments, 2, request);

    if (IsError(parse_result)) {
        return parse_result;
    }

    if (std::get_if<NilResult>(&parse_result)) {
        return ListResult{};
    }

    std::vector<GeoSearchValue> found = FindByRadius(*geo, request);
    PrepareGeoSearchResult(found, request);

    ListResult result;

    for (const auto& element: found) {
        result.value_.push_back(element.member_);
    }

    return result;
}

Result GeoSearchStore(DataBase& data_base, const std::vector<std::string>& arguments) {
    if (!CheckMinimalArguments(arguments, 7)) {
        return ErrorResult{"wrong number of arguments"};
    }

    const std::string& destination = arguments[1];
    const std::string& source = arguments[2];

    auto* value = data_base.GetElement(source);

    if (!value) {
        return IntegerResult{0};
    }

    auto* geo = std::get_if<GeoElement>(value);

    if (!geo) {
        return WrongType();
    }

    GeoSearchRequest request;

    Result parse_result = ParseGeoSearchRequest(*geo, arguments, 3, request);

    if (IsError(parse_result)) {
        return parse_result;
    }

    if (std::get_if<NilResult>(&parse_result)) {
        return IntegerResult{0};
    }

    std::vector<GeoSearchValue> found = FindByRadius(*geo, request);
    PrepareGeoSearchResult(found, request);

    GeoElement result_geo;

    for (const auto& element: found) {
        result_geo.Add(element.point_.longitude_, element.point_.latitude_, element.member_);
    }

    Result result = data_base.PutElement(destination, std::move(result_geo));

    if (IsError(result)) {
        return result;
    }

    return IntegerResult{static_cast<int64_t>(found.size())};
}