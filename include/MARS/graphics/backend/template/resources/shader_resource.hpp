#ifndef MARS_SHADER_RESOURCE_
#define MARS_SHADER_RESOURCE_

#include <MARS/resources/resource_manager.hpp>

namespace mars_graphics {

    class shader_resource : public mars_resources::resource_base{
    private:
        shader_core_data m_data;
        mars_ref<shader> m_shader;
        std::string m_render_type;
        std::string m_suffix;

        inline void set_shader_ref(const mars_ref<shader>& _ref) {
            m_shader = _ref;
        }

        friend shader_builder;
    public:
        shader_resource(const std::string& _suffix, const std::string& _render_type) {
            m_suffix = _suffix;
            m_render_type = _render_type;
        }

        [[nodiscard]] inline shader_core_data& data() {
            return m_data;
        }

        [[nodiscard]] inline mars_ref<shader> get_shader() const {
            return m_shader;
        }


        bool load_resource(const std::string &_path) override;
    };
}

#endif