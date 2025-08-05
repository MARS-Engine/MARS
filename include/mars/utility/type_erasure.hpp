#pragma once

#include <concepts>
#include <type_traits>

namespace mars {
    template <typename R, typename... Args>
        requires((std::is_reference_v<Args> || std::is_pointer_v<Args>) && ...)
    struct type_erased_fn {
      public:
        R (*m_ptr)(Args...);

        constexpr void clear() {
            m_ptr = nullptr;
        }

        template <typename... InArgs>
            requires((std::is_reference_v<InArgs> || std::is_pointer_v<InArgs>) && ...)
        constexpr void set(R (*_new_fn)(InArgs...)) {
            m_ptr = reinterpret_cast<R (*)(Args...)>(_new_fn);
        }

        constexpr R execute(Args... _args) const {
            if constexpr (std::is_same_v<R, void>) {
                if (!m_ptr)
                    return;
            } else if (!m_ptr)
                return {};

            if constexpr (std::is_same_v<R, void>)
                m_ptr(_args...);
            else
                return m_ptr(_args...);
        }
    };
} // namespace mars