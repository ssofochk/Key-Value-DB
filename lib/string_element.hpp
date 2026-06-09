#pragma once

#include <cstddef>
#include <string>

struct StringElement {
private:
    std::string value_;
public:

    StringElement();
    StringElement(const std::string& value);
    
    std::size_t Size() const;
    std::string Get() const;

    void Append(const std::string& other);
    std::size_t MemoryStored() const;
};