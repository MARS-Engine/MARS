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

using namespace mvre_graphics;

void v_backend_instance::create_with_window(const std::string &_title, mvre_math::vector2<int> _size) {
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

    m_render_pass = new v_render_pass(this);
    m_render_pass->create();

    m_framebuffer = new v_framebuffer(this);
    m_framebuffer->set_render_pass(m_render_pass);
    m_framebuffer->create(m_swapchain->image_views());

    m_command_pool = new v_command_pool(this);
    m_command_pool->create();

    auto main_buffer = new v_command_buffer(this);
    main_buffer->create();
    m_primary_buffer = main_buffer;

    m_sync = new v_sync(this);
    m_sync->create();
}

void v_backend_instance::update() {
    raw_window->process();
}

void v_backend_instance::prepare_render() {
    m_sync->wait();
    vkAcquireNextImageKHR(device()->raw_device(), swapchain()->raw_swapchain(), UINT64_MAX, sync()->image_available(), VK_NULL_HANDLE, &m_image_index);
    m_primary_buffer->reset();
    m_primary_buffer->begin();
    m_render_pass->begin();
}

void v_backend_instance::draw() {
    m_render_pass->end();
    m_primary_buffer->end();

    VkSemaphore wait_semaphores[] = { sync()->image_available() };
    VkSemaphore signal_semaphores[] = { sync()->render_finished() };
    VkPipelineStageFlags wait_Stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit_Info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_Stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &((v_command_buffer*)m_primary_buffer)->raw_command_buffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    if (vkQueueSubmit(device()->raw_graphics_queue(), 1, &submit_Info, sync()->inflight_fence()) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Backend Instance - Failed to submit draw command buffer");

    VkSwapchainKHR swapchains[] = { swapchain()->raw_swapchain() };

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &m_image_index,
    };

    vkQueuePresentKHR(device()->raw_present_queue(), &presentInfo);

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void v_backend_instance::destroy() {
    delete m_swapchain;

    delete m_device;

    ((v_window*)raw_window)->destroy_surface();
    delete m_instance;
    delete raw_window;
}