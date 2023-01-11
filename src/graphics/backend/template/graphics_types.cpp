#include <MARS/graphics/backend/template/graphics_types.hpp>

using namespace mars_graphics;

MARS_SHADER_TYPE graphics_types::token_to_type(MARS_SHADER_TOKEN _token) {
    switch (_token) {
        case MARS_SHADER_TOKEN_VERTEX:
            return MARS_SHADER_TYPE_VERTEX;
        case MARS_SHADER_TOKEN_FRAGMENT:
            return MARS_SHADER_TYPE_FRAGMENT;
        default:
            return MARS_SHADER_TYPE_UNDEFINED;
    }
}