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
        template <typename... Args>
        struct helper {
            // function used to pass extra arguments
            void (*helper_function)(const helper&, Args...) = nullptr;
            // function used for simple (runtime) callbacks
            void (*simple_function)(Args...) = nullptr;
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
                return std::meta::data_member_spec(
                    std::meta::substitute(^^std::vector, {
                                                             *std::views::single(t).data() }));
            }));
        }

        storage functions;
    };

    template <typename T>
    struct event : public event_storage<T> {
        template <auto Func, typename C, typename... Args>
        static void thunk(const event_storage<T>::template helper<Args...>& helper, Args&&... args) {
            if constexpr (std::tuple_size_v<typename mars::meta::function_pointer_info<decltype(Func)>::t_args_tuple> == sizeof...(Args))
                Func(args...);
            else
                Func(args..., *reinterpret_cast<C*>(helper.data));
        }

        template <auto Func, typename C, typename Tuple>
        struct thunk_from_tuple;

        template <auto Func, typename C, typename... Args>
        struct thunk_from_tuple<Func, C, std::tuple<Args...>> {
            static constexpr auto value = &thunk<Func, C, Args...>;
        };

        template <typename Tuple>
        struct signature_from_tuple;

        template <typename... Args>
        struct signature_from_tuple<std::tuple<Args...>> {
            typedef void (*signature)(Args...);
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
                if (function_vec[i].helper_function == thunk_from_tuple<F, C, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::value && function_vec[i].data == reinterpret_cast<uintptr_t>(_object))
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

        template <auto MemberPtr>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        bool has_bound() {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];
            return function_vec.size() != 0;
        }

        template <auto MemberPtr, auto F, typename C>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void listen(C& _object) {
            if (is_bound<MemberPtr, F, C>(_object))
                return;

            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];

            function_vec.push_back({ .helper_function = thunk_from_tuple<F, C, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::value, .data = reinterpret_cast<uintptr_t>(&_object) });
        }

        template <auto MemberPtr, auto F>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void listen() {
            if (is_bound<MemberPtr, F>())
                return;

            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];

            if constexpr (mars::meta::member_function_pointer_info<decltype(MemberPtr)>::args_size == mars::meta::function_pointer_info<decltype(F)>::args_size)
                function_vec.emplace_back().simple_function = F;
            else
                function_vec.push_back({ .helper_function = thunk_from_tuple<F, void, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::value });
        }

        template <auto MemberPtr>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void listen(signature_from_tuple<typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_args_tuple>::signature _function) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];

            function_vec.push_back({ .simple_function = _function });
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

        template <auto MemberPtr, typename... Args>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void broadcast(Args&&... args) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];

            for (int i = 0; i < function_vec.size(); i++) {
                if (function_vec[i].data == dead_mark) {
                    function_vec.erase(function_vec.begin() + i);
                    i--;
                    continue;
                }

                if (function_vec[i].simple_function)
                    function_vec[i].simple_function(std::forward<Args>(args)...);
                else
                    function_vec[i].helper_function(function_vec[i], std::forward<Args>(args)...);
            }
        }
    };

} // namespace mars