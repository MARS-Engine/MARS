#pragma once

#include <mars/debug/logger.hpp>
#include <mars/hash/meta.hpp>
#include <mars/meta.hpp>

#include <cstddef>
#include <meta>
#include <type_traits>

namespace mars::meta {
    namespace detail {
        inline static log_channel type_erasure_channel("mars/meta/type_erasure");
    } // namespace detail

    struct type_erased_ptr {
      private:
        struct empty {};

        void* data = nullptr;

        [[no_unique_address]]
        std::conditional_t<environment::is_shipping, empty, void (*)(const std::string_view&)> id;

        template <typename T>
        static void internal_error(const std::string_view& _value) {
            logger::assert_(detail::type_erasure_channel, "attempted to get type {} but {} is stored", _value, std::meta::display_string_of(^^T));
        }

      public:
        type_erased_ptr() {
            if (!environment::is_shipping)
                id = nullptr;
        }

        template <typename T>
        type_erased_ptr(T* _ptr) {
            store(_ptr);
        }

        template <typename T>
        void store(T* _ptr) {
            data = _ptr;

            if (!environment::is_shipping)
                id = &internal_error<T>;
        }

        template <typename T>
        T* get() const {
            if (!environment::is_shipping) {
                if (id == nullptr || id == &internal_error<T>)
                    return static_cast<T*>(data);
                id(std::meta::display_string_of(^^T));
                return nullptr;
            }
            return static_cast<T*>(data);
        }

        template <>
        inline void* get<void>() const {
            return data;
        }

        template <typename T>
        type_erased_ptr& operator=(T* _ptr) {
            store(_ptr);
            return *this;
        }

        template <typename T>
        operator T*() const {
            return get<T>();
        }
    };
} // namespace mars::meta