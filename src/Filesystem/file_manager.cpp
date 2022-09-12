#include "file_manager.hpp"
#include <fstream>

std::vector<std::string> FileManager::shader_locations() {
    return { "Engine/Assets/Shaders/", "Data/Shaders/" };
}

std::string FileManager::find_file(std::vector<std::string> _locations, std::string _file_location) {
    for (auto& l: _locations)
        if (file_exists(l + _file_location))
            return l + _file_location;

    return  "";
}

bool FileManager::file_exists(const std::string& _file_location) {
    std::ifstream file(_file_location.c_str());
    return file.is_open();
}

bool FileManager::read_binary_file(const std::string& _file_location, std::vector<uint32_t>& _data) {
    std::ifstream file(_file_location.c_str(), std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return false;

    size_t size = file.tellg();

    _data.resize(size / sizeof(uint32_t));

    file.seekg(0);
    file.read((char*)_data.data(), size);
    file.close();

    return true;
}

bool FileManager::read_file(const std::string& _file_location, std::vector<std::string> &_lines) {
    std::ifstream file(_file_location.c_str());

    if (!file.is_open())
        return false;

    std::string line;
    while (getline(file, line))
        _lines.push_back(line);

    return true;
}