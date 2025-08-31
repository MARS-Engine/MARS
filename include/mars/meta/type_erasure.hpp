#pragma once

#include <mars/debug/logger.hpp>
#include <mars/hash/meta.hpp>
#include <mars/meta.hpp>

#include <cstddef>
#include <meta>

namespace mars::meta {
    namespace detail {
        struct type_id;

        inline static log_channel type_erasure_channel("mars/meta/type_erasure");

        consteval {
            if (!environment::is_shipping) {
                std::meta::define_aggregate(^^type_id, {
                                                           std::meta::data_member_spec(^^size_t, {
                                                                                                     .name = "hash" }) });
            } else
                std::meta::define_aggregate(^^type_id, {
                                                       });
        };
    } // namespace detail

    struct type_erasure_ptr {
      private:
        void* data = nullptr;

        [[no_unique_address]]
        detail::type_id id;

      public:
        type_erasure_ptr() {
            if (!environment::is_shipping)
                id = { 0 };
        }

        template <typename T>
        type_erasure_ptr(T* _ptr) {
            store(_ptr);
        }

        template <typename T>
        void store(T* _ptr) {
            data = _ptr;

            if (!environment::is_shipping)
                id.hash = hash::type_fingerprint_v<T>;
        }

        template <typename T>
        T* get() const {
            if (!environment::is_shipping) {
                if (id.hash == 0 || id.hash == hash::type_fingerprint_v<T>)
                    return static_cast<T*>(data);
                logger::assert_(detail::type_erasure_channel, "attempted to get type {} but hash is {}", std::meta::identifier_of(^^T), id.hash);
                return nullptr;
            }
            return static_cast<T*>(data);
        }

        template <>
        inline void* get<void>() const {
            return data;
        }

        template <typename T>
        type_erasure_ptr& operator=(T* _ptr) {
            store(_ptr);
            return *this;
        }
    };
} // namespace mars::meta