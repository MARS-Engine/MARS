#include <MARS/graphics/backend/vulkan/vulkan_backend.hpp>
#include <MARS/graphics/backend/vulkan/v_window.hpp>
#include <MARS/graphics/backend/vulkan/v_render_pass.hpp>
#include <MARS/graphics/backend/vulkan/v_command_buffer.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_instance.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device_manager.hpp>
#include "MARS/graphics/backend/vulkan/v_swapchain.hpp"
#include <MARS/graphics/backend/vulkan/v_backend/v_sync.hpp>
#include "MARS/graphics/backend/vulkan/v_framebuffer.hpp"
#include <MARS/graphics/backend/vulkan/v_backend/v_command_pool.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_depth.hpp>
#include <MARS/graphics/light_manager.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

using namespace mars_graphics;

VkCommandBuffer vulkan_backend::get_single_time_command() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool()->raw_command_pool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device()->raw_device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void vulkan_backend::end_single_time_command(VkCommandBuffer _command) {
    vkEndCommandBuffer(_command);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_command;

    vkQueueSubmit(device()->raw_graphics_queue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device()->raw_graphics_queue());
    vkFreeCommandBuffers(device()->raw_device(), command_pool()->raw_command_pool(), 1, &_command);
}

void vulkan_backend::create_with_window(const std::string &_title, const mars_math::vector2<size_t>& _size, const std::string& _renderer) {
    raw_window = new v_window();
    raw_window->initialize(_title, _size);
    raw_window->create();

    m_vulkan_instance = new v_instance(this);
    m_vulkan_instance->create();

    get_vulkan_window()->create_surface(m_vulkan_instance);

    auto devices = get_vulkan_devices(this);

    if (devices.empty())
        mars_debug::debug::error("MARS - Vulkan - Device manager didn't return any device");

    m_device = devices[0];
    m_device->create();

    m_swapchain = new v_swapchain(shared_from_this());
    m_swapchain->create();

    m_renderer = new renderer(shared_from_this());
    m_renderer->create(_renderer);

    m_command_pool = new v_command_pool(this);
    m_command_pool->create();

    auto main_buffer = new v_command_buffer(shared_from_this());
    main_buffer->create();
    m_primary_buffer = main_buffer;

    m_sync = new v_sync(this);
    m_sync->create();

    m_light = new light_manager();
    m_light->load(m_graphics);
}

void vulkan_backend::update() {

}

void vulkan_backend::prepare_render() {
    m_sync->wait();
    vkAcquireNextImageKHR(device()->raw_device(), swapchain()->raw_swapchain(), UINT64_MAX, sync()->image_available(), VK_NULL_HANDLE, &m_index);
    vkResetFences(device()->raw_device(), 1, &sync()->inflight_fence());

    m_primary_buffer->reset();
    m_primary_buffer->begin();
    instance_renderer()->get_framebuffer("main_render")->get_render_pass()->begin();
}

void vulkan_backend::draw() {
    instance_renderer()->get_framebuffer("main_render")->get_render_pass()->end();
    m_light->draw_lights();
    m_primary_buffer->end();

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_Info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &sync()->image_available(),
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &((v_command_buffer*)m_primary_buffer)->raw_command_buffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &sync()->render_finished(),
    };

    if (vkQueueSubmit(device()->raw_graphics_queue(), 1, &submit_Info, sync()->inflight_fence()) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Backend Instance - Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &sync()->render_finished(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain()->raw_swapchain(),
        .pImageIndices = &m_index,
    };

    auto res = vkQueuePresentKHR(device()->raw_present_queue(), &presentInfo);
    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) //temp ignore suboptimal
        mars_debug::debug::error("MARS - Present Failed");

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    wait_idle();
}

void vulkan_backend::destroy() {
    m_light->destroy();
    delete m_light;

    m_swapchain->destroy();
    delete m_swapchain;

    ((v_command_buffer*)m_primary_buffer)->destroy();
    delete m_primary_buffer;

    m_command_pool->destroy();
    delete m_command_pool;

    m_sync->destroy();
    delete m_sync;

    instance_renderer()->destroy();
    delete m_renderer;

    m_shader_storage.lock()->clear();
    m_shader_input_storage.lock()->clear();
    m_pipeline_storage.lock()->clear();
    m_render_pass_storage.lock()->clear();
    m_shader_data_storage.lock()->clear();

    m_device->destroy();
    delete m_device;

    ((v_window*)raw_window)->destroy_surface();
    m_vulkan_instance->destroy();
    delete m_vulkan_instance;

    raw_window->destroy();
    delete raw_window;
}

void vulkan_backend::wait_idle() {
    vkDeviceWaitIdle(m_device->raw_device());
}