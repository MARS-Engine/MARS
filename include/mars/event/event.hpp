#pragma once

#include <cstddef>
#include <cstdint>
#include <mars/avx.hpp>
#include <mars/meta.hpp>
#include <meta>
#include <ranges>
#include <tuple>
#include <type_traits>

namespace mars {
    template <typename T>
    struct event_storage {

        // custom allocator for avx compatibility
        template <typename AT, std::size_t block_size>
        struct helper_allocator {
            using value_type = AT;

            // make rebind explicit so libstdc++ can keep block_size
            template <class U>
            struct rebind {
                using other = helper_allocator<U, block_size>;
            };

            helper_allocator() noexcept = default;

            template <class U>
            helper_allocator(const helper_allocator<U, block_size>&) noexcept {}

            AT* allocate(std::size_t n) {
                const std::size_t elems = ((n + block_size - 1) / block_size) * block_size;
                const std::size_t bytes = elems * sizeof(AT);
                return static_cast<AT*>(::operator new(bytes));
            }

            void deallocate(AT* p, std::size_t) noexcept {
                ::operator delete(p);
            }
        };

        template <typename... Args>
        struct helper {
            using helper_type = helper<Args...>;
            void (*function)(uintptr_t, Args...);
            uintptr_t data = 0;
        };

        struct storage;
        consteval {
            std::vector<std::meta::info> result = {};
            auto ctx = std::meta::access_context::current();

            // for each function in the type we extract the arguments and generate a meta::info helper with said arguments
            for (auto mem : std::meta::members_of(^^T, ctx)) {
                if (!std::meta::is_function(mem) || std::meta::is_special_member_function(mem))
                    continue;

                result.push_back(
                    std::meta::substitute(
                        ^^helper,
                        std::meta::parameters_of(mem) | std::views::transform(std::meta::type_of) | std::ranges::to<std::vector>()));
            }

            // define storage as struct { std::vector<helper<Args...>>, repeat for each function in T }
            std::meta::define_aggregate(^^storage, std::views::transform(result, [](auto t) {
                std::meta::info alloc_info = std::meta::substitute(^^helper_allocator, {
                                                                                           *std::views::single(t).data(), std::meta::reflect_constant(2) });
                return std::meta::data_member_spec(
                    std::meta::substitute(^^std::vector,
                                          {
                                              *std::views::single(t).data(), alloc_info }));
            }));
        }

        storage functions;
    };

    template <typename T>
    struct event : public event_storage<T> {
        template <auto Func, typename C, typename... Args>
        static void thunk(uintptr_t _ptr, Args... args) {
            if constexpr (std::tuple_size_v<typename mars::meta::member_pointer_info<decltype(Func)>::t_args_tuple> == sizeof...(Args))
                Func(args...);
            else
                Func(args..., *reinterpret_cast<C*>(_ptr));
        }

        template <auto Func, typename C, typename Tuple>
        struct thunk_from_tuple;

        template <auto Func, typename C, typename... Args>
        struct thunk_from_tuple<Func, C, std::tuple<Args...>> {
            static constexpr auto value = &thunk<Func, C, Args...>;
        };

        // if your object lives at 0x1 you are cooked.
        // I am not sacrificing AVX compatibility and 8 bytes per entry for you
        static constexpr uintptr_t dead_mark = 1;

        template <auto MemberPtr, auto F, typename C>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        bool internal_is_bound(uintptr_t _object) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];
            for (int i = 0; i < function_vec.size(); i++)
                if (function_vec[i].function == thunk_from_tuple<F, C, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::value && function_vec[i].data == reinterpret_cast<uintptr_t>(_object))
                    return true;
            return false;
        }

      public:
        template <auto MemberPtr, auto F, typename C>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        bool is_bound(C& _object) {
            return internal_is_bound<MemberPtr, F, C>(reinterpret_cast<uintptr_t>(&_object));
        }

        template <auto MemberPtr, auto F>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        bool is_bound() {
            return internal_is_bound<MemberPtr, F, void>(0);
        }

        template <auto MemberPtr, auto F, typename C>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void listen(C& _object) {
            if (is_bound<MemberPtr, F, C>(_object))
                return;

            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];

            for (int i = 0; i < function_vec.size(); i++)
                if (function_vec[i].data == reinterpret_cast<uintptr_t>(&_object))
                    return;

            function_vec.emplace_back(thunk_from_tuple<F, C, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::value, reinterpret_cast<uintptr_t>(&_object));
        }

        template <auto MemberPtr, auto F>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void listen() {
            if (is_bound<MemberPtr, F>())
                return;

            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];

            for (int i = 0; i < function_vec.size(); i++)
                if (function_vec[i].function == thunk_from_tuple<F, void, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::value)
                    return;

            function_vec.emplace_back(thunk_from_tuple<F, void, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::value, 0);
        }

        template <auto MemberPtr, auto F, typename C>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void stop_listening(C& _object) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];
            for (int i = 0; i < function_vec.size(); i++) {
                if (function_vec[i].data == reinterpret_cast<uintptr_t>(&_object)) {
                    function_vec[i].data = dead_mark;
                    break;
                }
            }
        }

        template <auto MemberPtr>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void broadcast(std::tuple_element_t<0, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple> first_arg, auto... args) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];
            for (int i = 0; i < function_vec.size(); i++) {
                if (function_vec[i].data == dead_mark) {
                    function_vec.erase(function_vec.begin() + i);
                    i--;
                    continue;
                }

                function_vec[i].function(function_vec[i].data, first_arg, args...);
            }
        }
    };

} // namespace mars