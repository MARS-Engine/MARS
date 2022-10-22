#include <MVRE/resources/resource_manager.hpp>
#include <fstream>

using namespace mvre_resources;
using namespace mvre_graphics_base;
using namespace pl;

safe_map<std::string, resource_base*> resource_manager::resources;

std::map<MVRE_RESOURCE_TYPE, std::string> resource_manager::resources_locations = {
        { MVRE_RESOURCE_TYPE_SHADER, "engine/assets/shaders/" }
};

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

void resource_manager::clean() {
    for (auto& kv : resources) {
        kv.second->clean();
        delete kv.second;
    }

    resources.clear();
    resources_locations.clear();
}