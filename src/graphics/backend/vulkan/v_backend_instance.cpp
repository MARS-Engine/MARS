#include <MVRE/graphics/backend/vulkan/v_backend_instance.hpp>
#include <MVRE/graphics/backend/vulkan/v_window.hpp>
#include <MVRE/graphics/backend/vulkan/v_render_pass.hpp>
#include <MVRE/graphics/backend/vulkan/v_command_buffer.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_instance.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device_manager.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_swapchain.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_sync.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_framebuffer.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_command_pool.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_depth.hpp>

using namespace mvre_graphics;

VkCommandBuffer v_backend_instance::get_single_time_command() {
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

void v_backend_instance::end_single_time_command(VkCommandBuffer _command) {
    vkEndCommandBuffer(_command);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_command;

    vkQueueSubmit(device()->raw_graphics_queue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device()->raw_graphics_queue());

    vkFreeCommandBuffers(device()->raw_device(), command_pool()->raw_command_pool(), 1, &_command);
}

void v_backend_instance::create_with_window(const std::string &_title, const mvre_math::vector2<int>& _size) {
    raw_window = new v_window();
    raw_window->initialize(_title, _size);
    raw_window->create();

    m_instance = new v_instance(this);
    m_instance->create();

    get_vulkan_window()->create_surface(m_instance);

    auto devices = get_vulkan_devices(this);

    if (devices.empty())
        mvre_debug::debug::error("MVRE - Vulkan - Device manager didn't return any device");

    m_device = devices[0];
    m_device->create();

    m_swapchain = new v_swapchain(this);
    m_swapchain->create();

    m_depth = new v_depth(this);
    m_depth->create();

    m_render_pass = new v_render_pass(this);
    m_render_pass->create();

    m_framebuffer = new v_framebuffer(this);
    m_framebuffer->set_render_pass(m_render_pass);
    m_framebuffer->create({ m_swapchain->extent().width, m_swapchain->extent().height }, m_swapchain->image_views());

    m_command_pool = new v_command_pool(this);
    m_command_pool->create();

    auto main_buffer = new v_command_buffer(this);
    main_buffer->create();
    m_primary_buffer = main_buffer;

    m_sync = new v_sync(this);
    m_sync->create();
}

void v_backend_instance::update() {

}

void v_backend_instance::prepare_render() {
    m_sync->wait();
    vkAcquireNextImageKHR(device()->raw_device(), swapchain()->raw_swapchain(), UINT64_MAX, sync()->image_available(), VK_NULL_HANDLE, &m_image_index);
    vkResetFences(device()->raw_device(), 1, &sync()->inflight_fence());

    m_primary_buffer->reset();
    m_primary_buffer->begin();
    m_render_pass->begin();
}

void v_backend_instance::draw() {
    m_render_pass->end();
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
        mvre_debug::debug::error("MVRE - Vulkan - Backend Instance - Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &sync()->render_finished(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain()->raw_swapchain(),
        .pImageIndices = &m_image_index,
    };

    if (vkQueuePresentKHR(device()->raw_present_queue(), &presentInfo) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Present Failed");
    vkQueueWaitIdle(device()->raw_present_queue());
    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void v_backend_instance::destroy() {
    m_render_pass->destroy();
    delete m_render_pass;

    m_depth->destroy();
    delete m_depth;

    m_swapchain->destroy();
    delete m_swapchain;

    ((v_command_buffer*)m_primary_buffer)->destroy();
    delete m_primary_buffer;

    m_command_pool->destroy();
    delete m_command_pool;

    m_sync->destroy();
    delete m_sync;

    m_framebuffer->destroy();
    delete m_framebuffer;

    m_device->destroy();
    delete m_device;

    ((v_window*)raw_window)->destroy_surface();
    m_instance->destroy();
    delete m_instance;
    delete raw_window;
}