#include "debug.hpp"
#include <iostream>

void debug::log(const char* message) {
    cout << "\033[32m[log] \033[0m" << message << endl;
}
void debug::log(const string& message) {
    log(message.c_str());
}

void debug::alert(const char* message) {

    cout << "\033[33m[alert] \033[0m" << message << endl;
}

void debug::alert(const string& message) {
    alert(message.c_str());
}

void debug::error(const char* message) {

    cout << "\033[31m[error] \033[0m" << message << endl;
    throw runtime_error("\n\033[31m[error] \033[0m" + (string)message);
}

void debug::error(const string& message) {
    error(message.c_str());
}