#include "MARS/engine/component.hpp"
#include <MARS/engine/engine_handler.hpp>

using namespace mars_engine;

float component::get_delta_time() { return object()->engine()->get_delta_time(); }
float component::get_delta_time_ms() { return object()->engine()->get_delta_time_ms(); }