#include "set_element.hpp"

#include <cstddef>
#include <string>
#include <vector>

int SetElement::Add(const std::string& value) {
    return value_.insert(value).second ? 1 : 0;
}

int SetElement::Remove(const std::string& value) {
    return static_cast<int>(value_.erase(value));
}

bool SetElement::Contains(const std::string& value) const {
    return value_.contains(value);
}

std::size_t SetElement::Size() const {
    return value_.size();
}

std::vector<std::string> SetElement::GetAll() const {
    std::vector<std::string> result;

    for (const auto& value: value_) {
        result.push_back(value);
    }

    return result;
}

void SetElement::Unite(const SetElement& other) {
    for (const auto& value: other.value_) {
        value_.insert(value);
    }
}

void SetElement::Intersect(const SetElement& other) {
    for (auto iterator = value_.begin(); iterator != value_.end();) {
        if (!other.Contains(*iterator)) {
            iterator = value_.erase(iterator);
        } else {
            ++iterator;
        }
    }
}

void SetElement::Diff(const SetElement& other) {
    for (const auto& value: other.value_) {
        value_.erase(value);
    }
}

std::size_t SetElement::MemoryStored() const {
    std::size_t ans = 0;

    for(const auto& element: value_) {
        ans += element.capacity();
    }

    return ans + sizeof(SetElement);
}