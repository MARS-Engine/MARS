#pragma once

#include <imgui.h>
#include <SDL3/SDL_events.h>
#include <cstddef>

namespace mars {
struct command_buffer;
struct device;
struct swapchain;
struct texture;
struct window;
} // namespace mars

namespace mars::imgui {

bool backend_supported(const mars::device& _device);
void initialize_backend(const mars::window& _window, const mars::device& _device, const mars::swapchain& _swapchain, size_t _frames_in_flight = 2);
void process_sdl_event(const SDL_Event& _event);
void new_frame();
ImTextureRef texture_ref(const mars::texture& _texture);
void release_texture_ref(const mars::texture& _texture);
void render_draw_data(const mars::command_buffer& _command_buffer);
void shutdown_backend();

} // namespace mars::imgui
