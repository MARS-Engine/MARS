#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include <mars/event/event.hpp>
#include <mars/graphics/backend/window.hpp>
#include <mars/math/vector2.hpp>
#include <vector>

namespace mars {
    struct graphics_engine;
    struct window;

    struct window_event_buttons {
        bool left_button;
        bool right_button;
    };

    struct window_event {
        void on_mouse_change(window&, const mars::vector2<size_t>& _position, const window_event_buttons& _click);
        void on_resize(window&, const mars::vector2<size_t>& _size);
        void on_close(window&);
    };

    struct window : event<window_event> {
        graphics_engine* engine;
        void* data;
        mars::vector2<size_t> size;
        SDL_Window* sdl_window = nullptr;
        Uint64 now = 0;
        Uint64 last = 0;
        float delta_time = 0.0f;
        SDL_WindowFlags flags = 0;

        inline void resize(const mars::vector2<size_t>& _size) {
            size = _size;
            broadcast<&window_event::on_resize>(*this, _size);
        }

        inline void mouse_change(const mars::vector2<size_t>& _position, const window_event_buttons& _click) {
            broadcast<&window_event::on_mouse_change>(*this, _position, _click);
        }

        inline void close() {
            broadcast<&window_event::on_close>(*this);
        }
    };

    namespace graphics {
        window window_create(graphics_engine& _engine, const window_params& _params);
        void window_create_surface(window& _window, instance& _instance);
        void window_get_instance_extensions(const window& _window, std::vector<const char*>& _out);
        void window_get_device_extensions(const window& _window, std::vector<const char*>& _out);
        void window_destroy_surface(window& _window, instance& _instance);
        void window_destroy(window& _window);
    } // namespace graphics

    inline void window_process_events(window& _window) {
        _window.now = SDL_GetPerformanceCounter();
        _window.delta_time = (_window.now - _window.last) / static_cast<float>(SDL_GetPerformanceFrequency());
        _window.last = _window.now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_Window* sdl_win = SDL_GetWindowFromID(event.window.windowID);
                if (sdl_win == _window.sdl_window)
                    _window.resize({ (size_t)event.window.data1, (size_t)event.window.data2 });
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                _window.mouse_change({ (size_t)event.motion.x, (size_t)event.motion.y }, { event.button.button == SDL_BUTTON_LEFT, event.button.button == SDL_BUTTON_RIGHT });
            } else if (event.type == SDL_EVENT_QUIT)
                _window.close();
        }
    }
} // namespace mars