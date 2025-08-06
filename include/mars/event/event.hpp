#pragma once

#include <mars/meta.hpp>
#include <ranges>

namespace mars {
    template <typename T>
    struct event_storage {
        template <typename... Args>
        struct helper {
            void (*function)(void*, Args...);
            void* data;
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
            std::meta::define_aggregate(^^storage,
                                        std::views::transform(result, [](auto t) {
                                            return std::meta::data_member_spec(
                                                std::meta::substitute(^^std::vector, std::views::single(t)));
                                        }));
        }

        storage functions;
    };

    template <typename T>
    struct event : public event_storage<T> {
        template <auto Func, typename C, typename... Args>
        static void thunk(void* _ptr, Args... args) {
            Func(args..., *static_cast<C*>(_ptr));
        }

        template <auto Func, typename C, typename Tuple>
        struct thunk_from_tuple;

        template <auto Func, typename C, typename... Args>
        struct thunk_from_tuple<Func, C, std::tuple<Args...>> {
            static constexpr auto value = &thunk<Func, C, Args...>;
        };

      public:
        template <auto MemberPtr, auto F, typename C>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void listen(C& ptr) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];

            for (int i = 0; i < function_vec.size(); i++)
                if (function_vec[i].data == &ptr)
                    return;

            function_vec.emplace_back(thunk_from_tuple<F, C, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::args_tuple>::value, &ptr);
        }

        template <auto MemberPtr, auto F, typename C>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void stop_listening(C& ptr) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            auto& function_vec = static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):];
            for (int i = 0; i < function_vec.size(); i++) {
                if (function_vec[i].data == &ptr) {
                    function_vec.erase(function_vec.begin() + i);
                    break;
                }
            }
        }

        template <auto MemberPtr, typename... Args>
            requires(std::is_same_v<T, typename mars::meta::member_function_pointer_info<decltype(MemberPtr)>::t_parent>)
        void broadcast(Args... args) {
            constexpr size_t index = mars::meta::get_member_function_position<MemberPtr>();
            for (auto& entry : static_cast<event_storage<T>&>(*this).functions.[:mars::meta::get_member_variable_by_index(^^decltype(event_storage<T>::functions), index):]) {
                entry.function(entry.data, args...);
            }
        }
    };

} // namespace mars