template <size_t MaxKeySize, size_t MaxValueSize>
std::optional<std::string> FileWorker<MaxKeySize, MaxValueSize>::read(const std::string& key) {
    if (key.length() > MaxKeySize || key.find(kDelimeter) != std::string::npos) {
        return std::nullopt;
    }

    std::shared_lock<std::shared_mutex> lock(fileMutex_);

    std::ifstream file(path_);
    if (!file.is_open()) return std::nullopt;

    std::string line;
    while (std::getline(file, line)) {
        size_t kDelimeterPos = line.find(kDelimeter);
        if (kDelimeterPos == std::string::npos) continue;

        std::string currentKey = line.substr(0, kDelimeterPos);
        if (currentKey == key) {
            return line.substr(kDelimeterPos + 1);
        }
    }
    return std::nullopt;
}

template <size_t MaxKeySize, size_t MaxValueSize>
void FileWorker<MaxKeySize, MaxValueSize>::write(const std::string& key, const std::string& value) {
    if (!ValidateLength(key, value) || !ValidateContent(key, value)) {
        throw std::invalid_argument("Invalid key/value: bounds exceeded or illegal characters found.");
    }

    std::unique_lock<std::shared_mutex> lock(fileMutex_);

    std::ifstream inFile(path_);
    std::stringstream buffer;
    bool updated = false;

    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            size_t kDelimeterPos = line.find(kDelimeter);
            if (kDelimeterPos != std::string::npos && line.substr(0, kDelimeterPos) == key) {
                buffer << key << kDelimeter << value << "\n";
                updated = true;
            } else if (!line.empty()) {
                buffer << line << "\n";
            }
        }
        inFile.close();
    }

    if (!updated) {
        buffer << key << kDelimeter << value << "\n";
    }

    std::ofstream outFile(path_, std::ios::trunc);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing.");
    }
    outFile << buffer.rdbuf();
}

template <size_t MaxKeySize, size_t MaxValueSize>
void FileWorker<MaxKeySize, MaxValueSize>::erase(const std::string& key) {
    if (key.length() > MaxKeySize || key.find(kDelimeter) != std::string::npos) {
        return;
    }

    std::unique_lock<std::shared_mutex> lock(fileMutex_);

    std::ifstream inFile(path_);
    if (!inFile.is_open()) return;

    std::stringstream buffer;
    std::string line;
    bool found = false;

    while (std::getline(inFile, line)) {
        size_t kDelimeterPos = line.find(kDelimeter);
        if (kDelimeterPos != std::string::npos && line.substr(0, kDelimeterPos) == key) {
            found = true;
            continue; 
        }
        if (!line.empty()) {
            buffer << line << "\n";
        }
    }
    inFile.close();

    if (found) {
        std::ofstream outFile(path_, std::ios::trunc);
        if (!outFile.is_open()) {
            throw std::runtime_error("Failed to open file for erasing.");
        }
        outFile << buffer.rdbuf();
    }
}