#ifndef __VFRAME__BUFFER__
#define __VFRAME__BUFFER__

#include "vtypes.hpp"
#include "Math/vector2.hpp"
#include <vector>



class vswapchain;
class vrender_pass;
class texture;

class vframebuffer {
public:
    std::vector<VkFramebuffer> raw_framebuffers;
    std::vector<VkImageView> attachments;

    vrender_pass* render_pass;

    void add_attachment(VkImageView _image_view);
    void set_attachments(std::vector<texture*> _textures);
    void clear_attachments();
    void create(vrender_pass* _render_pass, vector2 _size);
    void clean();
};

#endif