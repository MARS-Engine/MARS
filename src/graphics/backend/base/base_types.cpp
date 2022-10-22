#include "MVRE/graphics/backend/base/base_types.hpp"

using namespace mvre_graphics_base;

MVRE_SHADER_TYPE mvre_types::token_to_type(MVRE_SHADER_TOKEN _token) {
    switch (_token) {
        case MVRE_SHADER_TOKEN_VERTEX:
            return MVRE_SHADER_TYPE_VERTEX;
        case MVRE_SHADER_TOKEN_FRAGMENT:
            return MVRE_SHADER_TYPE_FRAGMENT;
        default:
            return MVRE_SHADER_TYPE_UNDEFINED;
    }
}