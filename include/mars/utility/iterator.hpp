#pragma once

#include <cstdint>

namespace mars {
    template <typename T>
    struct iterator {
      private:
        T* m_location;

      public:
        constexpr iterator() : m_location(nullptr) {}
        explicit constexpr iterator(T* _location) : m_location(_location) {}

        [[nodiscard]]
        constexpr T* ptr() const { return m_location; }
        [[nodiscard]]
        constexpr bool is_null() const { return m_location == nullptr; }

        [[nodiscard]]
        constexpr T* operator->() const { return m_location; }
        [[nodiscard]]
        constexpr T& operator*() const { return *m_location; }

        [[nodiscard]]
        constexpr size_t operator-(const iterator& _other) { return m_location - _other.m_location; }
        [[nodiscard]]
        constexpr iterator operator+(size_t _offset) { return iterator(m_location - _offset); }

        constexpr iterator& operator++() {
            ++m_location;
            return *this;
        }

        constexpr iterator& operator--() {
            --m_location;
            return *this;
        }

        [[nodiscard]]
        constexpr bool operator>=(const iterator& _other) const { return m_location >= _other.m_location; }
        [[nodiscard]]
        constexpr bool operator<=(const iterator& _other) const { return m_location <= _other.m_location; }
        [[nodiscard]]
        constexpr bool operator>(const iterator& _other) const { return m_location > _other.m_location; }
        [[nodiscard]]
        constexpr bool operator<(const iterator& _other) const { return m_location < _other.m_location; }
        [[nodiscard]]
        constexpr bool operator==(const iterator& _other) const { return m_location == _other.m_location; }
        [[nodiscard]]
        constexpr bool operator!=(const iterator& _other) const { return m_location != _other.m_location; }

        [[nodiscard]]
        constexpr T& operator[](size_t _index) const { return m_location[_index]; }
    };
} // namespace mars