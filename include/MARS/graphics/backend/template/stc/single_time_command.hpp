#ifndef MARS_SINGLE_TIME_COMMAND_
#define MARS_SINGLE_TIME_COMMAND_

#include <MARS/debug/debug.hpp>
#include <memory>

namespace mars_graphics {
    class graphics_backend;
    class command_buffer;

    class single_time_command {
    private:
        std::shared_ptr<graphics_backend> m_backend;
        bool m_executed = false;
        std::shared_ptr<command_buffer> m_command_buffer;
    protected:
        [[nodiscard]] std::shared_ptr<command_buffer> get_command_buffer() const {
            return m_command_buffer;
        }

        void base_execute();
    public:
        explicit single_time_command(const std::shared_ptr<graphics_backend>& _backend);

        single_time_command(const single_time_command& _copy) = delete;
        single_time_command(single_time_command&& _move) noexcept = default;
        single_time_command& operator=(const single_time_command& _copy) = delete;
        single_time_command& operator=(single_time_command&& _move) noexcept = default;

        ~single_time_command() {
            if (!m_executed)
                mars_debug::debug::alert("MARS - STC - Failed to execute command");
        }
    };
}

#endif