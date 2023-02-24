#include <MARS/resources/resource_manager.hpp>
#include <fstream>

using namespace mars_resources;
using namespace mars_graphics;
using namespace pl;

bool resource_manager::read_file(const std::string& _path, std::vector<std::string>& data) {
    std::ifstream stream(_path.c_str());

    if (!stream.is_open())
        return false;

    std::string line;
    while (std::getline(stream, line))
        data.push_back(line);

    stream.close();

    return true;
}

bool resource_manager::read_binary(const std::string& _path, std::vector<char>& data) {
    std::ifstream stream(_path.c_str(), std::ios::ate | std::ios::binary);

    if (!stream.is_open())
        return false;

    size_t file_size = (size_t)stream.tellg();
    data.resize(file_size);

    stream.seekg(0);
    stream.read(data.data(), file_size);

    stream.close();

    return true;
}

std::string resource_manager::find_path(const std::string& _file, mars_graphics::MARS_RESOURCE_TYPE _type, const std::string& _path_suffix) {
    return resources_locations[_type] + ( _path_suffix.empty() ? "" :  _path_suffix + "/") + _file;
}

void resource_manager::clean() {
    for (auto& kv : resources) {
        kv.second->clean();
        delete kv.second;
    }

    resources.clear();
    resources_locations.clear();
}