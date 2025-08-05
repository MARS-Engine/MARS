#pragma once

namespace mars::ui {

    enum length_type {
        LENGTH_TYPE_AUTO,
        LENGTH_TYPE_PIXEL,
        LENGTH_TYPE_PERCENTAGE
    };

    struct length {
        length_type type;
        float value;
    };
} // namespace mars::ui