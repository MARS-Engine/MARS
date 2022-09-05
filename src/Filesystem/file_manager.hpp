#ifndef __FILE__MANAGER__
#define __FILE__MANAGER__

#include <vector>
#include <string>

using namespace std;

class FileManager {
public:
    static vector<string> shader_locations();
    static string find_file(vector<string> _locations, string _file_location);
    static bool file_exists(const string& _file_location);
    static bool read_binary_file(const string& _file_location, vector<uint32_t>& _data);
    static bool read_file(const string& _file_location, vector<string>& _lines);
};


#endif