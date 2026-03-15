#pragma once

#include <string>

namespace mars::debug {

struct crash_data {
    std::string reason;
    std::string callstack;
};

using crash_handler_fn = void(*)(const crash_data&);

void set_crash_handler(crash_handler_fn handler);

void install_crash_handlers();

std::string capture_callstack();

} // namespace mars::debug
