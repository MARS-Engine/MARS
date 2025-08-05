#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace mars::io {
    inline std::string read_file(const std::string& _file_name) {
        std::ifstream file(_file_name);
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    inline bool file_exists(const std::string& path) {
        return std::filesystem::exists(path);
    }

    inline bool save_to_file(const std::string& file_name, const std::string& content) {
        std::ofstream file(file_name, std::ios::out | std::ios::trunc);
        if (!file.is_open())
            return false;

        file << content;
        return true;
    }

    inline std::vector<std::string> get_file_names_in_folder(const std::string& folder_path) {
        std::vector<std::string> file_names;

        for (const auto& entry : std::filesystem::directory_iterator(folder_path))
            if (entry.is_regular_file())
                file_names.push_back(entry.path().filename().string());

        return file_names;
    }

    inline std::vector<std::string> get_subdirectories(const std::string& parent_path) {
        std::vector<std::string> subdirectories;

        for (const auto& entry : std::filesystem::directory_iterator(parent_path))
            if (entry.is_directory())
                subdirectories.push_back(entry.path().filename().string());

        return subdirectories;
    }
} // namespace mars::io