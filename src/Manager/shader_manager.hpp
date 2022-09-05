#ifndef __SHADER__MANAGER__
#define __SHADER__MANAGER__

#include <vector>
#include <string>

using namespace std;

class shader;
class vengine;

class shader_manager {
public:
    static vector<shader*> shaders;

    static shader* get_shader(const string& path, vengine* engine);
    static void clean();
};

#endif