#include "component.hpp"
#include "engine_object.hpp"
#include "Manager/material_manager.hpp"

size_t component_interface::size() const { return sizeof(component_interface); }
void component_interface::pre_load() { }
void component_interface::load() { loaded = true; }
void component_interface::pre_render() { }
void component_interface::update() { }
void component_interface::render() { }
void component_interface::post_render() { }
void component_interface::clean() { }