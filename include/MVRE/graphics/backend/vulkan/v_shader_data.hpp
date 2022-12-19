#ifndef MVRE_V_SHADER_DATA_
#define MVRE_V_SHADER_DATA_

#include <MVRE/graphics/backend/template/shader_data.hpp>
#include <vulkan/vulkan.h>
#include "v_buffer.hpp"

namespace mvre_graphics {

    class v_uniform : uniform {
    public:
        inline VkBuffer get_buffer(size_t _index) { return ((v_buffer*)m_buffers[_index])->vulkan_buffer(); }

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

        void generate(pipeline* _pipeline, shader *_shader) override;
        void destroy() override;
    };
}

#endif