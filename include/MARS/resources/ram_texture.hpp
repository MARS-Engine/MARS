#ifndef MARS_RAM_TEXTURE_
#define MARS_RAM_TEXTURE_

#include "resource_manager.hpp"

namespace mars_resources {
    class ram_texture : public resource_base {
    private:
        std::string m_path;
        mars_math::vector2<int> m_size {};
        int m_channels = -1;
        unsigned char* m_data = nullptr;
    public:
        [[nodiscard]] int channels() const { return m_channels; }
        [[nodiscard]] const unsigned char* data() const { return m_data; }

        [[nodiscard]] inline mars_math::vector2<int> size() const { return m_size; }

        inline mars_math::vector4<unsigned char> get_pixel(size_t x, size_t y) const {
            size_t pos = x + y * m_size.x;
            return { m_data[pos], m_data[pos + 1], m_data[pos + 2], m_data[pos + 3] };
        }

        bool load_resource(const std::string &_path) override;

        void clean() override;
    };
}

#endif