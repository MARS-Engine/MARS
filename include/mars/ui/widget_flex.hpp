#pragma once

#include "length.hpp"

#include <cstdint>
#include <mars/math/vector2.hpp>
#include <mars/math/vector3.hpp>
#include <mars/math/vector4.hpp>

namespace mars::ui {
    enum flex_layout {
        FLEX_LAYOUT_FLEX,
        FLEX_LAYOUT_ABSOLUTE
    };

    enum flex_direction {
        FLEX_DIRECTION_ROW,
        FLEX_DIRECTION_ROW_REVERSE,
        FLEX_DIRECTION_COLUMN,
        FLEX_DIRECTION_COLUMN_REVERSE
    };

    enum flex_wrap {
        FLEX_WRAP_WRAP,
        FLEX_WRAP_NOWRAP,
        FLEX_WRAP_REVERSE
    };

    enum flex_justify_content {
        FLEX_JUSTIFY_CONTENT_FLEX_START,
        FLEX_JUSTIFY_CONTENT_FLEX_END,
        FLEX_JUSTIFY_CONTENT_CENTER,
        FLEX_JUSTIFY_CONTENT_SPACE_BETWEEN,
        FLEX_JUSTIFY_CONTENT_SPACE_AROUND,
        FLEX_JUSTIFY_CONTENT_SPACE_EVENLY
    };

    enum flex_align_items {
        FLEX_ALIGN_ITEMS_FLEX_START,
        FLEX_ALIGN_ITEMS_FLEX_END,
        FLEX_ALIGN_ITEMS_CENTER,
        FLEX_ALIGN_ITEMS_STRETCH,
        FLEX_ALIGN_ITEMS_BASELINE,
    };

    enum flex_align_content {
        FLEX_ALIGN_CONTENT_FLEX_START,
        FLEX_ALIGN_CONTENT_FLEX_END,
        FLEX_ALIGN_CONTENT_CENTER,
        FLEX_ALIGN_CONTENT_STRETCH,
        FLEX_ALIGN_CONTENT_SPACE_BETWEEN,
        FLEX_ALIGN_CONTENT_SPACE_AROUND,
        FLEX_ALIGN_CONTENT_SPACE_EVENLY
    };

    /**
     * data:
     * x - top
     * y - right
     * z - bottom
     * w - left
     *
     * rect:
     * xy - pos
     * zw - size
     */
    struct widget_flex {
        vector4<float> margin;
        vector4<float> border;
        vector4<float> padding;

        // only used when paret layout is absolute
        vector2<float> size;
        vector2<float> position;

        vector3<float> colour;

        flex_layout layout;
        flex_direction direction;

        length gap;
        float groth;

        std::string content;

        vector4<float> get_child_rect(const vector4<float>& _parent_rect, size_t _children) {
            vector4<float> result = _parent_rect;

            if (groth) {
                switch (direction) {
                case FLEX_DIRECTION_ROW:
                    result.z /= (float)_children;
                    break;
                case FLEX_DIRECTION_COLUMN:
                    result.w /= (float)_children;
                    break;
                default:
                    break;
                }
            } else
                result.zw() + size;

            return result;
        }

        void apply_render_changes(vector4<float>& _rect) {
            _rect.xy() += { margin.w, margin.x };
            _rect.zw() += padding.xy() + padding.zw();
        }

        void apply_offset(widget_flex& _parent_flex, vector2<float>& _current_offset, const vector4<float>& _rect) {
            switch (_parent_flex.direction) {
            case FLEX_DIRECTION_ROW:
                _current_offset.x += _rect.z;
                _current_offset.x += margin.y;
                break;
            case FLEX_DIRECTION_COLUMN:
                _current_offset.y += _rect.w;
                _current_offset.y += margin.z;
                break;
            default:
                break;
            }
        }
    };
} // namespace mars::ui