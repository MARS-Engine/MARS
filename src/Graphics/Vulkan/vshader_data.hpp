#ifndef __VSHADER__DATA__
#define __VSHADER__DATA__

#include "vtypes.hpp"
#include <vector>
#include <map>
#include <string>

using namespace std;

struct vuniform_data;
class vdescriptor_pool;
class VShader;
class vtexture;
class vbuffer;
class vdevice;
class vcommand_buffer;
class vpipeline;

struct vuniform {
    vuniform_data* data;
    vtexture* texture;
    vector<vbuffer*> uniform;
    vdevice* device;
    VmaAllocator allocator;

    vuniform(vuniform_data* _data, vdevice* _device, VmaAllocator& _allocator);
    void generate(size_t _buffer_size, size_t _framebuffer_size);
    void update(void* _data, size_t _index);
    void set_texture(vtexture* _texture);
};

class vshader_data {
private:
    size_t size;

    void update_descriptors();
public:
    vector<vuniform*> uniforms;
    VShader* shader;
    VkDescriptorSetLayout descriptor_set_layout;
    vdescriptor_pool* descriptor_pool;
    vector<VkDescriptorSet> descriptor_sets;
    VmaAllocator allocator;

    vshader_data(VShader* _shader, VmaAllocator& _allocator);
    vuniform* get_uniform(string _name);
    void change_texture(string _name, vtexture* _texture);
    void bind(vcommand_buffer* _command_buffer, vpipeline* _pipeline);
    void generate(size_t _size);
};

#endif