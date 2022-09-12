#ifndef __FILE__MANAGER__
#define __FILE__MANAGER__

#include <vector>
#include <string>



class FileManager {
public:
    static std::vector<std::string> shader_locations();
    static std::string find_file(std::vector<std::string> _locations, std::string _file_location);
    static bool file_exists(const std::string& _file_location);
    static bool read_binary_file(const std::string& _file_location, std::vector<uint32_t>& _data);
    static bool read_file(const std::string& _file_location, std::vector<std::string>& _lines);
};


#endif