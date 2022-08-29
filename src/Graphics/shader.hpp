#ifndef __SHADER__
#define __SHADER__

#include <string>

using namespace std;

class vengine;
class VShader;

class shader {
public:
    vengine* engine;
    VShader* base_shader;
    string location;

    shader(vengine* _engine);
    void load_shader(const string& _location);
};

#endif