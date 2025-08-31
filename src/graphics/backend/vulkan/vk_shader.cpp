#include <mars/graphics/backend/vulkan/vk_shader.hpp>

#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/io/file.hpp>
#include <mars/meta.hpp>

#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {

    namespace detail {
        sparse_vector<vk_shader, 6> shaders;
        log_channel shader_channel("graphics/vulkan/shader");

        bool create_shader_module(VkDevice _device, const shader_module& _module, vk_shader_module& _out) {
            std::vector<char> file_data;

            if (!io::read_file(_module.path, file_data)) {
                logger::error(shader_channel, "failed to open file when creating shader module | path - {}", _module.path);
                return false;
            }

            VkShaderModuleCreateInfo create_info{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = file_data.size(),
                .pCode = reinterpret_cast<const uint32_t*>(file_data.data()),
            };

            VkResult result = vkCreateShaderModule(_device, &create_info, nullptr, &_out.module);
            logger::error_if(result != VK_SUCCESS, shader_channel, "failed to create shader module due to error {} | shader path - {}", meta::enum_to_string(result), _module.path);

            _out.type = _module.type;

            return result == VK_SUCCESS;
        }
    } // namespace detail

    shader vk_shader_impl::vk_shader_create(device& _device, const std::vector<shader_module>& _shaders) {
        vk_shader* shader_ptr = detail::shaders.request_entry();
        vk_device* device_ptr = _device.data.get<vk_device>();

        shader result;
        result.data = shader_ptr;
        result.engine = _device.engine;

        shader_ptr->modules.resize(_shaders.size());

        for (int i = 0; i < _shaders.size(); i++)
            detail::create_shader_module(device_ptr->device, _shaders[i], shader_ptr->modules[i]);

        return result;
    }

    void vk_shader_impl::vk_shader_destroy(shader& _shader, const device& _device) {
        vk_shader* shader_ptr = _shader.data.get<vk_shader>();
        vk_device* device_ptr = _device.data.get<vk_device>();

        for (vk_shader_module& module : shader_ptr->modules)
            vkDestroyShaderModule(device_ptr->device, module.module, nullptr);

        detail::shaders.remove(shader_ptr);
        _shader = {};
    }
} // namespace mars::graphics::vulkan