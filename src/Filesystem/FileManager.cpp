#include "FileManager.hpp"
#include <fstream>

string FileManager::ShaderLocation() {
    return "Engine/Assets/Shaders/";
}

bool FileManager::FileExists(const string& fileLocation) {
    ifstream file(fileLocation.c_str());
    return file.is_open();
}

bool FileManager::ReadBinaryFile(const string& fileLocation, vector<uint32_t>& data) {
    ifstream file(fileLocation.c_str(), std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return false;

    size_t size = file.tellg();

    data.resize(size / sizeof(uint32_t));

    file.seekg(0);
    file.read((char*)data.data(), size);
    file.close();

    return true;
}

bool FileManager::ReadFile(const string& fileLocation, vector<std::string> &lines) {
    ifstream file(fileLocation.c_str());

    if (!file.is_open())
        return false;

    string line;
    while (getline(file, line))
        lines.push_back(line);

    return true;
}