#include <mars/graphics/backend/vulkan/vk_window.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_instance.hpp>
#include <mars/graphics/window.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <meta>
#include <vector>

namespace mars::graphics::vulkan {
    namespace detail {
        log_channel win_channel("graphics/vulkan/window");
        sparse_vector<vk_window, 2> windows;
    } // namespace detail

    window vk_window_impl::vk_window_create(graphics_engine& _engine, const window_params& _params) {
        window new_window;
        new_window.engine = &_engine;
        new_window.data = detail::windows.request_entry();

        if (!SDL_WasInit(SDL_INIT_VIDEO))
            SDL_Init(SDL_INIT_VIDEO);

        new_window.sdl_window = SDL_CreateWindow(_params.title.c_str(), _params.size.x, _params.size.y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

        new_window.now = SDL_GetPerformanceCounter();
        new_window.last = new_window.now;
        return new_window;
    }

    void vk_window_impl::vk_window_create_surface(window& _window, instance& _instance) {
        vk_window* window_ptr = static_cast<vk_window*>(_window.data);
        vk_instance* instance_ptr = static_cast<vk_instance*>(_instance.data);
        bool surface_result = SDL_Vulkan_CreateSurface(_window.sdl_window, instance_ptr->instance, nullptr, &window_ptr->surface);

        logger::assert_(surface_result, detail::win_channel, "failed to create surface from sdl window");
    }

    void vk_window_impl::vk_window_get_extensions(const window& _window, std::vector<const char*>& _extensions) {
        Uint32 count_instance_extensions;
        const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

        if (instance_extensions == nullptr) {
            logger::error(detail::win_channel, "failed get extensions of sdl3 window");
            return;
        }

        for (size_t i = 0; i < count_instance_extensions; i++)
            _extensions.push_back(instance_extensions[i]);
    }

    void vk_window_impl::vk_window_destroy_surface(window& _window, instance& _instance) {
        vk_window* window_ptr = static_cast<vk_window*>(_window.data);
        vk_instance* instance_ptr = static_cast<vk_instance*>(_instance.data);

        logger::assert_(window_ptr->surface != VK_NULL_HANDLE, detail::win_channel, "error destroying surface handle is null");

        vkDestroySurfaceKHR(instance_ptr->instance, window_ptr->surface, nullptr);
    }

    void vk_window_impl::vk_window_destroy(window& _window) {
        vk_window* window_ptr = static_cast<vk_window*>(_window.data);

        SDL_DestroyWindow(_window.sdl_window);

        int windows_left = 0;
        SDL_GetWindows(&windows_left);

        if (windows_left == 0)
            SDL_Quit();

        _window = {};

        detail::windows.remove(window_ptr);
    }
} // namespace mars::graphics::vulkan