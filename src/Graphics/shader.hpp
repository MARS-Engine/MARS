#ifndef __SHADER__
#define __SHADER__

#include <string>



class vengine;
class VShader;

class shader {
private:
    std::string render_type_to_path();
public:
    vengine* engine;
    VShader* base_shader;
    std::string location;

    shader(vengine* _engine);
    void load_shader(const std::string& _location);
};

#endif