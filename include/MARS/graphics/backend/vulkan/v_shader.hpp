#ifndef MARS_V_SHADER_
#define MARS_V_SHADER_

#include <MARS/graphics/backend/template/shader.hpp>
#include <vulkan/vulkan.h>

namespace mars_graphics {

    class v_buffer;

    class v_shader : public shader {
    private:
        std::map<MARS_SHADER_TYPE, VkShaderModule> m_v_modules;
        std::map<MARS_SHADER_TYPE, unsigned int> m_shaders_modules;

        VkDescriptorSetLayout m_uniform_layout;

        std::vector<VkDescriptorPoolSize> m_pool_descriptors;

        void generate_shader(MARS_SHADER_TYPE _type, const std::string& _data) override;

        bool load_shader(const mars_ref<mars_graphics::shader_resource> &_resource) override;

        std::string get_suffix() override { return ".spv"; }
    public:

        std::vector<VkPipelineShaderStageCreateInfo> get_stages();

        inline std::vector<VkDescriptorPoolSize>& get_pool_descriptors() { return m_pool_descriptors; }

        inline VkDescriptorSetLayout& raw_uniform_layout() { return m_uniform_layout; }

        using shader::shader;

        ~v_shader();
    };
}

#endif