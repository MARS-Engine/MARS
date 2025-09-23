#pragma once

#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {

    struct imgui_initialize_params {
        window g_window;
        instance g_instance;
        device g_device;
        descriptor g_descriptor;
        render_pass g_render_pass;
        size_t min_image;
        size_t image_count;
    };

    void imgui_initialize(imgui_initialize_params _params);
    void imgui_render(const command_buffer& _command_buffer);
} // namespace mars