#include "debug.hpp"
#include <iostream>

void debug::log(const char* message) {
    std::cout << "\033[32m[log] \033[0m" << message << std::endl;
}
void debug::log(const std::string& message) {
    log(message.c_str());
}

void debug::alert(const char* message) {

    std::cout << "\033[33m[alert] \033[0m" << message << std::endl;
}

void debug::alert(const std::string& message) {
    alert(message.c_str());
}

void debug::error(const char* message) {

    std::cout << "\033[31m[error] \033[0m" << message << std::endl;
    throw std::runtime_error("\n\033[31m[error] \033[0m" + (std::string)message);
}

void debug::error(const std::string& message) {
    error(message.c_str());
}