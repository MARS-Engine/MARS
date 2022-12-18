#ifndef __MVRE__V__SHADER__
#define __MVRE__V__SHADER__

#include <MVRE/graphics/backend/template/shader.hpp>
#include <vulkan/vulkan.h>

namespace mvre_graphics {

    class v_buffer;

    class v_shader : public shader {
    private:
        std::map<MVRE_SHADER_TYPE, VkShaderModule> m_v_modules;
        std::map<MVRE_SHADER_TYPE, unsigned int> m_shaders_modules;

        VkDescriptorSetLayout m_uniform_layout;
        VkDescriptorPool m_descriptor_pool;

        void generate_shader(MVRE_SHADER_TYPE _type, const std::string& _data) override;
    public:
        std::vector<VkPipelineShaderStageCreateInfo> get_stages();

        inline VkDescriptorSetLayout raw_uniform_layout() { return m_uniform_layout; }
        inline VkDescriptorPool raw_descriptor_pool() { return m_descriptor_pool; }

        using shader::shader;

        bool load_resource(const std::string& _path) override;
    };
}

#endif