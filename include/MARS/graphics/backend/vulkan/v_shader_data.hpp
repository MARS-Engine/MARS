#ifndef MARS_V_SHADER_DATA_
#define MARS_V_SHADER_DATA_

#include <MARS/graphics/backend/template/shader_data.hpp>
#include <vulkan/vulkan.h>
#include "v_buffer.hpp"

namespace mars_graphics {

    class v_uniform : public uniform {
    public:
        inline VkBuffer& get_buffer(size_t _index) { return std::static_pointer_cast<v_buffer>(m_buffer)->vulkan_buffer(); }

        using uniform::uniform;

        void destroy() override;
    };

    class v_shader_data : public shader_data {
    private:
        std::vector<VkDescriptorSet> m_descriptor_sets;
        VkDescriptorPool m_descriptor_pool;
    public:
        using shader_data::shader_data;

        void bind() override;

        void generate(const std::shared_ptr<pipeline>& _pipeline, const std::shared_ptr<shader>& _shader) override;
        void destroy() override;
    };
}

#endif