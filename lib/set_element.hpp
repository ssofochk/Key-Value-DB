#pragma once

#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>

struct SetElement {
private:
    std::unordered_set<std::string> value_;

public:
    int Add(const std::string& value);
    int Remove(const std::string& value);
    bool Contains(const std::string& value) const;

    std::vector<std::string> GetAll() const;
    void Unite(const SetElement& other);
    void Intersect(const SetElement& other);
    void Diff(const SetElement& other);

    std::size_t Size() const;
    std::size_t MemoryStored() const;
};