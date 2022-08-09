#ifndef __SHADER__
#define __SHADER__

#include <string>

using namespace std;

class VEngine;
class VShader;

class Shader {
public:
    VEngine* engine;
    VShader* vShader;

    Shader(VEngine* engine);
    void LoadShader(const string& location);
};

#endif