#include <mars/graphics/backend/vulkan/window.hpp>

#include <mars/debug/logger.hpp>
#include <mars/graphics/window.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <meta>
#include <vector>

namespace mars::graphics::vulkan {
    namespace detail {
        mars::log_channel win_channel("graphics/vulkan/window");
    }

    window vk_window_impl::window_create(graphics_engine& _engine, const window_params& _params) {
        window new_window;
        new_window.engine = &_engine;

        if (!SDL_WasInit(SDL_INIT_VIDEO))
            SDL_Init(SDL_INIT_VIDEO);

        new_window.sdl_window = SDL_CreateWindow(_params.title.c_str(), _params.size.x, _params.size.y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

        new_window.now = SDL_GetPerformanceCounter();
        new_window.last = new_window.now;
        return new_window;
    }

    void vk_window_impl::get_extensions(const window& _window, std::vector<const char*>& _extensions) {
        Uint32 count_instance_extensions;
        const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

        if (instance_extensions == nullptr) {
            logger::error(detail::win_channel, "failed get extensions of sdl3 window");
            return;
        }

        for (size_t i = 0; i < count_instance_extensions; i++)
            _extensions.push_back(instance_extensions[i]);
    }
} // namespace mars::graphics::vulkan