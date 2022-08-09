#ifndef __FILEMANAGER__
#define __FILEMANAGER__

#include <vector>
#include <string>

using namespace std;

class FileManager {
public:
    static string ShaderLocation();
    static bool FileExists(const string& fileLocation);
    static bool ReadBinaryFile(const string& fileLocation, vector<uint32_t>& data);
    static bool ReadFile(const string& fileLocation, vector<string>& lines);
};


#endif