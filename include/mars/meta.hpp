#pragma once

#include <array>
#include <meta>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace mars::meta {
    template <class>
    struct member_pointer_info;

    template <class C, class T>
    struct member_pointer_info<T C::*> {
        using parent = C;
        using type = T;
    };

    template <class>
    struct member_function_pointer_info;

    template <class C, class R, class... Args>
    struct member_function_pointer_info<R (C::*)(Args...)> {
        using t_parent = C;
        using t_return = R;
        using t_args_tuple = std::tuple<Args...>;
        inline static constexpr size_t args_size = sizeof...(Args);
    };

    template <class>
    struct function_pointer_info;

    template <class R, class... Args>
    struct function_pointer_info<R (*)(Args...)> {
        using t_return = R;
        using t_args_tuple = std::tuple<Args...>;
        inline static constexpr size_t args_size = sizeof...(Args);
    };

    template <typename Tuple, typename Extra>
    struct append_to_tuple;

    template <typename... Ts, typename Extra>
    struct append_to_tuple<std::tuple<Ts...>, Extra> {
        using type = std::tuple<Ts..., Extra>;
    };

    template <typename R, typename Tuple>
    struct unpack_to_function;

    template <typename R, typename... Args>
    struct unpack_to_function<R, std::tuple<Args...>> {
        using type = R (*)(Args...);
    };

    namespace detail {
        template <auto MemberPtr, typename Parent, size_t I = 0>
        consteval size_t member_index_impl() {
            constexpr auto ctx = std::meta::access_context::current();

            constexpr size_t N =
                std::meta::nonstatic_data_members_of(^^Parent, ctx).size();

            if constexpr (I == N)
                return -1;
            else {
                constexpr auto M =
                    std::meta::nonstatic_data_members_of(^^Parent, ctx)[I];

                using ptr_t = decltype(MemberPtr);

                if constexpr (std::meta::is_same_type(std::meta::type_of(M), ^^typename member_pointer_info<ptr_t>::type))
                    if (std::meta::extract<ptr_t>(M) == MemberPtr)
                        return I;

                return member_index_impl<MemberPtr, Parent, I + 1>();
            }
        }
    } // namespace detail

    template <std::size_t I, typename Spec>
    using template_arg_t = [:std::meta::template_arguments_of(^^Spec)[I]:];

    template <typename T>
    consteval bool is_type(std::meta::info _type) {
        return std::meta::is_same_type(std::meta::type_of(_type), ^^T);
    }

    template <typename T>
    consteval std::optional<T> get_annotation(std::meta::info _type) {
        for (auto& annotation : std::meta::annotations_of(_type))
            if (std::meta::is_same_type(^^T, std::meta::type_of(annotation)))
                return { std::meta::extract<T>(annotation) };
        return {};
    }

    template <typename T>
    consteval bool has_annotation(std::meta::info _type) {
        for (auto& annotation : std::meta::annotations_of(std::meta::type_of(_type)))
            if (std::meta::is_same_type(^^T, std::meta::type_of(annotation)))
                return true;
        return false;
    }

    template <auto MemberPtr>
    consteval size_t get_member_position() {
        return detail::member_index_impl<MemberPtr, typename member_pointer_info<decltype(MemberPtr)>::parent>();
    }

    template <typename E, size_t N = std::meta::enumerators_of(^^E).size()>
        requires std::is_enum_v<E>
    consteval std::array<const char*, N> enum_values() {
        std::array<const char*, N> result;
        size_t i = 0;
        for (auto e : std::meta::enumerators_of(^^E))
            result.at(i++) = std::meta::identifier_of(e).begin();
        return result;
    }

    template <typename E, bool Enumerable = std::meta::is_enumerable_type(^^E)>
        requires std::is_enum_v<E>
    constexpr std::string_view enum_to_string(E _value) {
        if constexpr (Enumerable) {
            template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E))) {
                if (_value == [:e:])
                    return std::meta::identifier_of(e);
            }
        }

        return "<unnamed>";
    }

    template <typename E, bool Enumerable = std::meta::is_enumerable_type(^^E)>
        requires std::is_enum_v<E>
    constexpr E string_to_enum(const std::string_view& _value) {
        if constexpr (Enumerable) {
            template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E))) {
                if (_value == std::meta::identifier_of(e))
                    return [:e:];
            }
        }

        return {};
    }

    template <auto MemberPtr>
    consteval size_t get_member_function_position() {
        constexpr auto ctx = std::meta::access_context::current();
        using parent_type = member_function_pointer_info<decltype(MemberPtr)>::t_parent;

        std::size_t index = 0;

        template for (constexpr auto mem : std::define_static_array(std::meta::members_of(^^parent_type, ctx))) {
            if constexpr (!std::meta::is_function(mem) || std::meta::is_special_member_function(mem))
                continue;
            else {
                auto current_ptr = &[:mem:];
                if constexpr (std::is_same_v<decltype(current_ptr),
                                             decltype(MemberPtr)>) {
                    if (current_ptr == MemberPtr)
                        return index; // found it
                }
            }
            ++index;
        }
        return index;
    }

    consteval std::meta::info get_member_variable_by_index(std::meta::info r, std::size_t n) {
        return nonstatic_data_members_of(r, std::meta::access_context::current())[n];
    }

} // namespace mars::meta