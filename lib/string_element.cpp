#include "string_element.hpp"

#include <cstddef>
#include <string>

StringElement::StringElement() {}

StringElement::StringElement(const std::string& value) :
    value_(value) {}

std::size_t StringElement::Size() const {
    return value_.size();
}

std::string StringElement::Get() const {
    return value_;
}

void StringElement::Append(const std::string& other) {
    value_.append(other);
}

std::size_t StringElement::MemoryStored() const {
    return sizeof(StringElement) + value_.capacity();
}