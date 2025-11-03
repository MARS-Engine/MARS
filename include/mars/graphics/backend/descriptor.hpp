#pragma once

#include <mars/graphics/backend/buffer_view.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/graphics/backend/texture.hpp>

#include <array>
#include <meta>
#include <vector>

enum mars_descriptor_type {
    MARS_DESCRIPTOR_TYPE_SAMPLER,
    MARS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    MARS_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    MARS_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    MARS_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    MARS_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
    MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
    MARS_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
};

namespace mars {
    struct descriptor {
        graphics_backend_functions* engine;
        meta::type_erased_ptr data;
        size_t frames_in_flight;
    };

    struct descriptor_create_params {
        std::array<size_t, std::meta::enumerators_of(^^mars_descriptor_type).size()> descriptors_size = {};
        size_t max_sets;
    };

    struct descriptor_set {
        graphics_backend_functions* engine;
        meta::type_erased_ptr data;
    };

    struct descriptor_set_create_params {
        std::vector<std::pair<buffer, size_t>> buffers;
        std::vector<std::pair<buffer_view, size_t>> buffer_views;
        std::vector<std::pair<texture, size_t>> textures;
    };

    struct descriptor_impl {
        descriptor (*descriptor_create)(const device& _device, const descriptor_create_params& _params, size_t _frames_in_flight) = nullptr;
        descriptor_set (*descriptor_set_create)(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) = nullptr;
        void (*descriptor_set_bind)(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame);
        void (*descriptor_destroy)(descriptor& _descriptor, const device& _device) = nullptr;
    };
} // namespace mars