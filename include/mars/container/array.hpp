#pragma once

#include <stddef.h>

namespace mars {
    namespace detail {
        template <typename A>
        constexpr decltype(auto) dive(A& _array) noexcept { return _array; }

        template <typename A, typename... I>
        constexpr decltype(auto) dive(A& _array, size_t _i0, I... _is) noexcept {
            return dive(_array[_i0], _is...);
        }

        template <typename T, size_t... S>
        struct make_array;

        template <typename T, size_t F, size_t... S>
        struct make_array<T, F, S...> {
            using type = typename make_array<T, S...>::type[F];
        };

        template <typename T>
        struct make_array<T> {
            using type = T;
        };
    }; // namespace detail

    template <typename T, size_t... S>
    struct array {
        detail::make_array<T, S...>::type data;

        template <typename... I>
        auto& operator[](I... _index) {
            return detail::dive(data, _index...);
        }

        template <typename... I>
        const auto& operator[](I... _index) const {
            return detail::dive(data, _index...);
        }
    };
} // namespace mars