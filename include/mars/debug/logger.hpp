#pragma once

#include <format>
#include <stdexcept>
#include <utility>

#ifndef IS_SHIPPING
#define IS_SHIPPING false
#endif
#ifndef IS_DEBUG
#define IS_DEBUG true
#endif
#ifndef SAFETY_LEVEL
#define SAFETY_LEVEL 0
#endif

namespace mars {
    class log_channel {
      private:
        std::string m_channel_name;

      public:
        [[nodiscard]]
        inline const std::string& name() const { return m_channel_name; }

        log_channel(const std::string& _name) {
            m_channel_name = _name;
        }

        log_channel(const char* _name) {
            m_channel_name = _name;
        }
    };

    namespace environment {
        /**
         * Level of safety when building
         */
        enum BUILD_SAFETY_LEVEL {
            BUILD_SAFETY_LEVEL_NONE = 0,
            BUILD_SAFETY_LEVEL_LOW = 1,
            BUILD_SAFETY_LEVEL_MEDIUM = 2,
            BUILD_SAFETY_LEVEL_HIGH = 3
        };

        constexpr bool is_shipping = IS_SHIPPING;
        constexpr bool is_debug = IS_DEBUG;
        constexpr unsigned int safety_level = SAFETY_LEVEL;

        static_assert(is_shipping != is_debug, "IS_SHIPPING or IS_DEBUG incorrectly set");
    } // namespace environment

    namespace logger {
        template <typename... Args>
        void mars_print(const std::string& _level, const log_channel& _channel, const std::string& _message, Args... args) {
            std::printf("[%s] | %s | %s\n", _level.c_str(), _channel.name().c_str(), std::vformat(std::string_view(_message.c_str()), std::make_format_args(args...)).c_str());
        }

        template <typename... Args>
        void log(const log_channel& _channel, const std::string& _message, Args... args) {
            mars_print("log", _channel, _message, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void warning(const log_channel& _channel, const std::string& _message, Args... args) {
            mars_print("warning", _channel, _message, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void error(const log_channel& _channel, const std::string& _message, Args... args) {
            mars_print("error", _channel, _message, std::forward<Args>(args)...);
        }

        template <typename... Args>
        [[noreturn]]
        void assert_(const log_channel& _channel, const std::string& _message, Args... args) {
            mars_print("assert", _channel, _message, std::forward<Args>(args)...);
            throw;
        }

        template <typename... Args>
        void assert_if(const bool _condition, const log_channel& _channel, const std::string& _message, Args... args) {
            if (_condition)
                assert_(_channel, _message, std::forward<Args>(args)...);
        }
    } // namespace logger
} // namespace mars