#include "list_element.hpp"

#include <cstddef>
#include <iterator>
#include <string>

void ListElement::PushFront(const std::string& value) {
    value_.push_front(value);
}

void ListElement::PushBack(const std::string& value) {
    value_.push_back(value);
}

std::string ListElement::PopFront() {
    std::string value = value_.front();
    value_.pop_front();
    return value;

}

std::string ListElement::PopBack() {
    std::string value = value_.back();
    value_.pop_back();
    return value;
}

bool ListElement::Empty() const {
    return value_.empty();
}

std::size_t ListElement::Size() const {
    return value_.size();
}

int ListElement::Insert(const std::string& pos, const std::string& pivot, const std::string& value) {
    for (auto iterator = value_.begin(); iterator != value_.end(); ++iterator) {
        if (*iterator == pivot) {
            if (pos == "BEFORE") {
                value_.insert(iterator, value);
            } else {
                value_.insert(std::next(iterator), value);
            }

            return static_cast<int>(value_.size());
        }
    }

    return -1;
}

std::string& ListElement::operator[](int index) {
    return value_[index];
}

const std::string& ListElement::operator[](int index) const {
    return value_[index];
}

std::size_t ListElement::MemoryStored() const {
    std::size_t ans = 0;

    for (const auto& element: value_) {
        ans += element.capacity();
    }

    return ans + sizeof(ListElement);
}