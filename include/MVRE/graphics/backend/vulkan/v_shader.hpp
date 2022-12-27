#ifndef MVRE_V_SHADER_
#define MVRE_V_SHADER_

#include <MVRE/graphics/backend/template/shader.hpp>
#include <vulkan/vulkan.h>

namespace mvre_graphics {

    class v_buffer;

    class v_shader : public shader {
    private:
        std::map<MVRE_SHADER_TYPE, VkShaderModule> m_v_modules;
        std::map<MVRE_SHADER_TYPE, unsigned int> m_shaders_modules;

        VkDescriptorSetLayout m_uniform_layout;

        std::vector<VkDescriptorPoolSize> m_pool_descriptors;

        void generate_shader(MVRE_SHADER_TYPE _type, const std::string& _data) override;
    public:
        std::vector<VkPipelineShaderStageCreateInfo> get_stages();

        inline std::vector<VkDescriptorPoolSize>& get_pool_descriptors() { return m_pool_descriptors; }

        inline VkDescriptorSetLayout& raw_uniform_layout() { return m_uniform_layout; }

        using shader::shader;

        bool load_resource(const std::string& _path) override;

        void clean() override;
    };
}

#endif