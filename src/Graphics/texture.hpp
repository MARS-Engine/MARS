#ifndef __TEXTURE__
#define __TEXTURE__

class vengine;
class command_buffer;
class pipeline;

#include "Vulkan/vtypes.hpp"
#include "Vulkan/vtexture.hpp"
#include "Math/vector2.hpp"
#include <string>

using namespace std;

class texture {
public:
    vtexture* base_texture;
    vengine* engine;
    string location;

    inline vector2 get_size() const { return base_texture->size; }

    texture(vengine* _engine);
    void load_texture(const string& _location);
    void create(vector2 _size, VkFormat _format, VkImageUsageFlagBits _usage);
    void bind(command_buffer* _command_buffer, pipeline* _pipeline) const;

};

#endif