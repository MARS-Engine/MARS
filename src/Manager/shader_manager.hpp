#ifndef __SHADER__MANAGER__
#define __SHADER__MANAGER__

#include <vector>
#include <string>



class shader;
class vengine;

class shader_manager {
public:
    static std::vector<shader*> shaders;

    static shader* get_shader(const std::string& path, vengine* engine);
    static void clean();
};

#endif