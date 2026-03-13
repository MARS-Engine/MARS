#pragma once

#include <SDL3/SDL_events.h>
#include <cstddef>

namespace mars {
struct command_buffer;
struct device;
struct swapchain;
struct window;
} // namespace mars

namespace mars::imgui {

bool backend_supported(const mars::device& device);
void initialize_backend(const mars::window& window, const mars::device& device, const mars::swapchain& swapchain, size_t frames_in_flight = 2);
void process_sdl_event(const SDL_Event& event);
void new_frame();
void render_draw_data(const mars::command_buffer& command_buffer);
void shutdown_backend();

} // namespace mars::imgui
