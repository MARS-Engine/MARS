#ifndef __FILE__MANAGER__
#define __FILE__MANAGER__

#include <vector>
#include <string>

using namespace std;

class FileManager {
public:
    static vector<string> ShaderLocations();
    static string FindFile(vector<string> locations, string fileLocation);
    static bool FileExists(const string& fileLocation);
    static bool ReadBinaryFile(const string& fileLocation, vector<uint32_t>& data);
    static bool ReadFile(const string& fileLocation, vector<string>& lines);
};


#endif