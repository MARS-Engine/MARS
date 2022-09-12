#include "vframebuffer.hpp"
#include "vswapchain.hpp"
#include "vrender_pass.hpp"
#include "vdevice.hpp"
#include "vdepth.hpp"
#include "vtexture.hpp"

void vframebuffer::add_attachment(VkImageView _image_view) {
    attachments.push_back(_image_view);
}

void vframebuffer::set_attachments(std::vector<texture*> _textures) {
    attachments.resize(_textures.size());

    for (auto i = 0; i < _textures.size(); i++)
        attachments[i] = _textures[i]->base_texture->image_view;
}

void vframebuffer::clear_attachments() {
    attachments.clear();
}

void vframebuffer::create(vrender_pass* _render_pass, vector2 _size) {
    render_pass = _render_pass;

    VkFramebufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.renderPass = render_pass->raw_render_pass;
    bufferInfo.pAttachments = attachments.data();
    bufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    bufferInfo.width = static_cast<uint32_t>(_size.x);
    bufferInfo.height = static_cast<uint32_t>(_size.y);
    bufferInfo.layers = 1;

    VkFramebuffer buffer;
    VK_CHECK(vkCreateFramebuffer(render_pass->device->raw_device, &bufferInfo, nullptr, &buffer));
    raw_framebuffers.push_back(buffer);
}

void vframebuffer::clean() {
    for (auto& rawFramebuffer : raw_framebuffers)
        vkDestroyFramebuffer(render_pass->device->raw_device, rawFramebuffer, nullptr);
}