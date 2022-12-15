#ifndef __MVRE__DEBUG__
#define __MVRE__DEBUG__

#include <iostream>

namespace mvre_debug {

    class debug {
    public:
        static inline void log(const char* message) { std::cout << "\033[32m[log] \033[0m" << message << std::endl; }
        static inline void log(const std::string& message) { log(message.c_str()); }

        static inline void alert(const char* message) { std::cout << "\033[33m[alert] \033[0m" << message << std::endl; }
        static inline void alert(const std::string& message) { alert(message.c_str()); }

        [[noreturn]] static inline void error(const char* message) { std::cout << "\033[31m[error] \033[0m" << message << std::endl; throw std::runtime_error("\033[31m[error] \033[0m" + (std::string)message); }
        [[noreturn]] static inline void error(const std::string& message) { error(message.c_str()); }
    };
}

#endif