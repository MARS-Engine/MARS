#include "file_manager.hpp"
#include <fstream>

vector<string> FileManager::shader_locations() {
    return { "Engine/Assets/Shaders/", "Data/Shaders/" };
}

string FileManager::find_file(vector<string> _locations, string _file_location) {
    for (auto& l: _locations)
        if (file_exists(l + _file_location))
            return l + _file_location;

    return  "";
}

bool FileManager::file_exists(const string& _file_location) {
    ifstream file(_file_location.c_str());
    return file.is_open();
}

bool FileManager::read_binary_file(const string& _file_location, vector<uint32_t>& _data) {
    ifstream file(_file_location.c_str(), std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return false;

    size_t size = file.tellg();

    _data.resize(size / sizeof(uint32_t));

    file.seekg(0);
    file.read((char*)_data.data(), size);
    file.close();

    return true;
}

bool FileManager::read_file(const string& _file_location, vector<std::string> &_lines) {
    ifstream file(_file_location.c_str());

    if (!file.is_open())
        return false;

    string line;
    while (getline(file, line))
        _lines.push_back(line);

    return true;
}