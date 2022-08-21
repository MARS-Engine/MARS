#ifndef __SHADERMANAGER__
#define __SHADERMANAGER__

#include <vector>
#include <string>

using namespace std;

class Shader;
class VEngine;

class ShaderManager {
public:
    static vector<Shader*> shaders;

    static Shader* GetShader(const string& path, VEngine* engine);
    static void Clean();
};

#endif