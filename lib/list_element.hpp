#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

struct ListElement {
private:
    std::deque<std::string> value_;

public:
    void PushFront(const std::string& value);
    void PushBack(const std::string& value);
    std::string PopFront();
    std::string PopBack();

    bool Empty() const;
    std::size_t Size() const;

    int Insert(const std::string& pos, const std::string& pivot, const std::string& value);

    std::string& operator[](int index);
    const std::string& operator[](int index) const;

    std::size_t MemoryStored() const;
};