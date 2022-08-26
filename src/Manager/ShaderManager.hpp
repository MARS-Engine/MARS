#ifndef __SHADER__MANAGER__
#define __SHADER__MANAGER__

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