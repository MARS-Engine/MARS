#include "vengine.hpp"
#include "Vulkan/vdevice.hpp"
#include "Vulkan/vinstance.hpp"
#include "Vulkan/vsurface.hpp"
#include "Vulkan/vswapchain.hpp"
#include "Vulkan/vcommand_pool.hpp"
#include "Vulkan/vrender_pass.hpp"
#include "Vulkan/vframebuffer.hpp"
#include "Vulkan/vdepth.hpp"
#include "Vulkan/vsync.hpp"
#include "Vulkan/vbuffer.hpp"
#include "Manager/render_pass_manager.hpp"
#include "Manager/material_manager.hpp"
#include "Components/Graphics/camera.hpp"
#include "Renderer/deferred_renderer.hpp"
#include "Renderer/simple_renderer.hpp"
#include "window.hpp"

unsigned int vengine::frame_overlap = 2;

Camera* vengine::get_camera() {
    if (cameras.empty())
        return nullptr;
    return cameras[0];
}

void vengine::create_base() {
    instance = new vinstance();
    instance->create();

    surface = new vsurface(surf_window, instance);
    surface->create();

    device = new vdevice(instance, surface);
    device->create();

    allocator = vbuffer::GenerateAllocator(device, instance);

    swapchain = new vswapchain(device);
    swapchain->create();

    frame_overlap = swapchain->image_views.size();

    commandPool = new vcommand_pool(device);
    commandPool->create();

    sync = new vsync(device);
    sync->create();

    renderPass = render_pass_manager::get_render_pass("default", this, {.should_load = false, .swapchain_ready = true});
    renderPass->prepare({});
    renderPass->add_description(swapchain->format);
    renderPass->add_depth(surf_window->size);
    renderPass->create();

    framebuffer = new vframebuffer();

    for (auto view : swapchain->image_views) {
        framebuffer->add_attachment(view);
        framebuffer->add_attachment(renderPass->depth->image_view);
        framebuffer->create(renderPass, swapchain->size);
        framebuffer->clear_attachments();
    }
}

void vengine::create(RENDER_TYPE _type, window* _window) {
    surf_window = _window;
    type = _type;

    create_base();

    switch (type) {
        case SIMPLE:
            renderer = new simple_renderer(this);
            renderer->load();
            break;
        case DEFERRED:
            renderer = new deferred_renderer(this);
            renderer->create_texture("deferredPosition", POSITION);
            renderer->create_texture("deferredNormal", POSITION);
            renderer->create_texture("deferredAlbedo", COLOR);
            renderer->create_texture("deferredMetal", COLOR);
            renderer->create_texture("deferredRough", COLOR);
            renderer->create_texture("deferredAO", COLOR);
            renderer->load();
            break;
    }
}

VkFramebuffer vengine::get_framebuffer(int i) {
    return renderer->get_framebuffer(i);
}

void vengine::prepare_draw() {
    renderer->update();
    sync->wait(render_frame);
    VK_CHECK(vkAcquireNextImageKHR(device->raw_device, swapchain->raw_swapchain, 1000000000, sync->presents[render_frame], nullptr, &image_index));
    renderer->clear();

}

void vengine::draw() {
    auto nonTransSize = draw_queue.size();
    draw_queue.resize(nonTransSize + trans_queue.size());

    sort(trans_queue.begin(), trans_queue.end(), [](auto l, auto r) { return l.distance > r.distance; });

    auto i = 0;
    for (auto t : trans_queue)
        draw_queue[nonTransSize + i++] = t.buffer;

    if (renderer)
        renderer->render();

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &sync->presents[render_frame];

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &sync->renders[render_frame];

    submit.commandBufferCount = draw_queue.size();
    submit.pCommandBuffers = draw_queue.data();

    VK_CHECK(vkQueueSubmit(device->graphics_queue, 1, &submit, sync->fences[render_frame]));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &swapchain->raw_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &sync->renders[render_frame];
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &image_index;

    VK_CHECK(vkQueuePresentKHR(device->graphics_queue, &presentInfo));

    render_frame = (render_frame + 1) % frame_overlap;
    trans_queue.resize(0);
    draw_queue.resize(0);
}

void vengine::clean() const {
    commandPool->clean();
    renderPass->clean();
    framebuffer->clean();
    swapchain->clean();
    device->clean();
    surface->clean();
    instance->clean();
}