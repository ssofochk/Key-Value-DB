#include <filesystem>
#include <string>
#include <shared_mutex>
#include <optional>
#include <fstream>
#include <sstream>
#include <stdexcept>

template <size_t MaxKeySize, size_t MaxValueSize>
class FileWorker {
public:

    explicit FileWorker(std::filesystem::path path)
        : path_(std::move(path)) {}

    std::optional<std::string> read(const std::string& key);

    void write(
        const std::string& key,
        const std::string& value
    );

    void erase(const std::string& key);

private:
    std::filesystem::path path_;
    mutable std::shared_mutex fileMutex_;

    static constexpr char kDelimeter = '\x1F';

    bool ValidateLength(const std::string& key, const std::string& value) const {
        return key.length() <= MaxKeySize && value.length() <= MaxValueSize;
    }

    bool ValidateContent(const std::string& key, const std::string& value) const {
        if (key.find(kDelimeter) != std::string::npos || value.find(kDelimeter) != std::string::npos) {
            return false;
        }
        if (key.find('\n') != std::string::npos || value.find('\n') != std::string::npos) {
            return false;
        }
        return true;
    }
    
};


#include "fileworker.tpp"