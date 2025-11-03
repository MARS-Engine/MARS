#pragma once

#include "type_erased.hpp"

namespace mars::meta {

    template <typename T>
    concept is_function_ptr = std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>;

    struct type_erased_fn {
      private:
        struct empty {};

        std::byte data[sizeof(void (*)())]{};

        [[no_unique_address]]
        std::conditional_t<environment::is_shipping, empty, void (*)(const std::string_view&)> id;

        template <typename T>
        static void internal_error(const std::string_view& _value) {
            logger::assert_(detail::type_erasure_channel, "attempted to get type {} but {} is stored", _value, std::meta::display_string_of(^^T));
        }

      public:
        type_erased_fn() {
            if (!environment::is_shipping)
                id = nullptr;
        }

        template <typename T>
            requires is_function_ptr<T>
        type_erased_fn(T _ptr) {
            store(_ptr);
        }

        template <typename T>
            requires is_function_ptr<T>
        void store(T _ptr) {
            static_assert(sizeof(T) == sizeof(data), "somehow T is bigger than void(*)(), if the constraint is working correctly you are on your own");
            std::memcpy(data, &_ptr, sizeof(T));

            if (!environment::is_shipping)
                id = &internal_error<T>;
        }

        template <typename T>
            requires is_function_ptr<T>
        T get() const {
            T result;
            if (!environment::is_shipping) {
                if (id != nullptr && id != &internal_error<T>) {
                    id(std::meta::display_string_of(^^T));
                    return nullptr;
                }
            }

            std::memcpy(&result, data, sizeof(T));

            return result;
        }

        template <typename R, typename... Args>
        R operator()(Args... args) const {
            R (*fn)(Args...) = get<R (*)(Args...)>();
            return fn(std::forward<Args>(args)...);
        }

        template <typename T>
            requires is_function_ptr<T>
        type_erased_fn& operator=(T _ptr) {
            store(_ptr);
            return *this;
        }

        template <typename T>
            requires is_function_ptr<T>
        operator T() const {
            return get<T>();
        }
    };
} // namespace mars::meta