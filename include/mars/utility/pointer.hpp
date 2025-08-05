#pragma once

#include <cstdint>

namespace mars {
    template <typename T>
    class pointer {
      private:
        uintptr_t m_ptr = 0;

        T* get_unmarked_ptr() const { return reinterpret_cast<T*>(m_ptr & ~1); }

      public:
        pointer() = default;
        pointer(T* _ptr) : m_ptr(reinterpret_cast<uintptr_t>(_ptr)) {}

        [[nodiscard]]
        bool is_marked() const { return (m_ptr & 1) == true; }
        void mark() { m_ptr |= 1; }

        T* get() { return get_unmarked_ptr(); }

        T* operator->() const { return get_unmarked_ptr(); }

        pointer& operator=(T* _ptr) {
            const bool marked = is_marked();

            m_ptr = reinterpret_cast<uintptr_t>(_ptr);

            if (marked)
                mark();

            return *this;
        }

        bool operator==(const T* _ptr) const { return get_unmarked_ptr() == _ptr; }
        bool operator==(const pointer _ptr) const { return get_unmarked_ptr() == _ptr.get_unmarked_ptr(); }
        T& operator*() const { return *get_unmarked_ptr(); }
    };
} // namespace mars