#include <filesystem>
#include <string>
#include <shared_mutex>

class FileWorker {
public:
    explicit FileWorker(std::filesystem::path path)
        : path_(std::move(path)) {}

    template <typename T>
    T read(const std::string& key);

    template <typename T>
    void write(
        const std::string& key,
        const T value
    );

    void erase(const std::string& key);

private:
    std::filesystem::path path_;
    mutable std::shared_mutex fileMutex_;
};

#include "fileworker.tpp"