#pragma once

#include <cctype>
#include <format>
#include <mars/math/vector3.hpp>
#include <mars/meta.hpp>
#include <mars/parser/parser.hpp>
#include <mars/utility/hex.hpp>
#include <meta>
#include <type_traits>

namespace mars::json {

    struct json_skip_anoation {};

    static constexpr json_skip_anoation skip = {};

    enum json_value_types {
        JSON_VALUE_TYPES_INVALID,
        JSON_VALUE_TYPES_STRING,
        JSON_VALUE_TYPES_OBJECT,
        JSON_VALUE_TYPES_NUMBER,
        JSON_VALUE_TYPES_ARRAY,
        JSON_VALUE_TYPES_BOOL,
    };

    inline json_value_types
    predict_value(unsigned char _value) {
        if (_value == '{')
            return JSON_VALUE_TYPES_OBJECT;
        if (_value == '[')
            return JSON_VALUE_TYPES_ARRAY;
        if (_value == '"')
            return JSON_VALUE_TYPES_STRING;
        if (std::isdigit(_value))
            return JSON_VALUE_TYPES_NUMBER;
        if (std::tolower(_value) == 't' || std::tolower(_value) == 'f')
            return JSON_VALUE_TYPES_BOOL;
        return JSON_VALUE_TYPES_INVALID;
    }

    template <typename T>
    struct json_type_parser_base {
        static constexpr bool array_support = false;
        static constexpr bool string_support = false;
        static constexpr bool bool_support = false;
        static constexpr bool number_support = false;
        static constexpr bool struct_support = false;
    };

    template <typename T>
    struct json_type_parser : public json_type_parser_base<T> {
        inline static std::string_view::iterator parse(const std::string_view& _json, T& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());

            if (*start != '{')
                return _json.end();

            std::string_view::iterator current = start + 1;

            do {
                current = parse::first_space<false>(current, _json.end());

                if (*current == '}')
                    return current;

                else if (current == _json.end())
                    return _json.end();

                if (*current != '"')
                    return _json.end();

                std::string name = parse::extract_string(current, _json.end());

                current += 2 + name.size();

                // check if next is :
                current = parse::first_space<false>(current, _json.end());

                if (*current != ':')
                    return _json.end();

                current = parse::first_space<false>(current + 1, _json.end());

                if (current == _json.end())
                    return _json.end();

                // there are no guarantees that the value is valid, it simply predicts based on the first character
                json_value_types predicted_type = predict_value(*current);

                if (predicted_type == JSON_VALUE_TYPES_INVALID)
                    return _json.end();

                static constexpr auto ctx = std::meta::access_context::current();

                bool found = false;

                if constexpr (std::is_class_v<T>) {
                    template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                        if (std::define_static_string(std::meta::identifier_of(mem)) != name)
                            continue;

                        bool valid = false;
                        using C = typename[:std::meta::type_of(mem):];
                        switch (predicted_type) {
                        case JSON_VALUE_TYPES_STRING:
                            valid = json_type_parser<C>::string_support;
                            break;
                        case JSON_VALUE_TYPES_OBJECT:
                            valid = json_type_parser<C>::struct_support;
                            break;
                        case JSON_VALUE_TYPES_NUMBER:
                            valid = json_type_parser<C>::number_support;
                            break;
                        case JSON_VALUE_TYPES_ARRAY:
                            valid = json_type_parser<C>::array_support;
                            break;
                        case JSON_VALUE_TYPES_BOOL:
                            valid = json_type_parser<C>::bool_support;
                            break;
                        default:
                            return _json.end();
                        }

                        std::string_view str{ current, _json.end() };
                        if (valid) {
                            found = true;
                            current = json_type_parser<C>::parse(str, _value.[:mem:]);
                        }
                    }
                }

                if (!found) {
                    // slip missing entry we assume the first , outside a object and string
                    size_t object_count = 0;
                    bool string_count;
                    while (current != _json.end() && (object_count != 0 || string_count || (*current != ',' && *current != '}'))) {
                        current = parse::first_special(current, _json.end());
                        if (*current == '{')
                            object_count++;
                        if (*current == '"') {
                            if (string_count && *(current - 1) != '\\')
                                string_count = false;
                            else
                                string_count = true;
                        } else if (*current == '}' && object_count) {
                            object_count--;
                            current++;
                        }

                        if (string_count || object_count || (*current != '}' && *current != ','))
                            current++;
                    }
                    if (current == _json.end()) {
                        return current;
                    }
                } else {
                    current = parse::first_space<false>(current, _json.end());
                }

                if (*current == ',') {
                    current++;
                } else if (*current != '}') {
                    return _json.end();
                }
            } while (*current != '}');

            return current + 1;
        }

        inline static void stringify(T& _value, std::string& _out) {
            _out += "{ ";
            static constexpr auto ctx = std::meta::access_context::current();
            if constexpr (std::is_class_v<T>) {
                template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                    constexpr std::optional<json_skip_anoation> annoation = mars::meta::get_annotation<json_skip_anoation>(mem);
                    if constexpr (!annoation.has_value()) {
                        _out += '"' + std::string(std::define_static_string(std::meta::identifier_of(mem))) + "\":";
                        json_type_parser<typename[:std::meta::type_of(mem):]>::stringify(_value.[:mem:], _out);
                        _out += ',';
                    }
                }
            }
            _out[_out.size() - 1] = '}';
        }

        static constexpr bool struct_support = true;
    };

    template <typename T>
    struct json_type_parser<std::vector<T>> : public json_type_parser_base<std::vector<T>> {
        inline static std::string_view::iterator parse(const std::string_view& _json, std::vector<T>& _value) {
            std::string_view::iterator current = parse::first_space<false>(_json.begin(), _json.end());

            if (*current != '[')
                return _json.end();

            current++;

            while (current != _json.end() && *current != ']') {
                current = parse::first_space<false>(current, _json.end());

                if (*current != '{')
                    return _json.end();

                std::string_view str{ current, _json.end() };

                if (json_type_parser<T>::struct_support)
                    current = json_type_parser<T>::parse(str, _value.emplace_back());

                if (current == _json.end())
                    return _json.end();

                current = parse::first_space<false>(current, _json.end());

                if (*current == ',')
                    current++;
                else if (*current != ']')
                    return _json.end();
            }

            return current;
        }

        inline static void stringify(std::vector<T>& _value, std::string& _out) {
            _out += "[ ";

            for (T& entry : _value) {
                json_type_parser<T>::stringify(entry, _out);
                _out += ",";
            }

            _out[_out.size() - 1] = ']';
        }

        static constexpr bool array_support = true;
    };

    template <>
    struct json_type_parser<unsigned int> : public json_type_parser_base<unsigned int> {
        inline static std::string_view::iterator parse(const std::string_view& _json, unsigned int& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
            std::string_view::iterator end = parse::first_special(start, _json.end());

            if (*end != ',' && *end != '}')
                return _json.end();

            std::string str(start, end);
            for (char& c : str)
                if (!std::isdigit(c))
                    return _json.end();
            _value = std::stoul(str);
            return end++;
        }

        inline static void stringify(unsigned int& _value, std::string& _out) {
            _out += std::to_string(_value);
        }

        static constexpr bool number_support = true;
    };

    template <>
    struct json_type_parser<size_t> : public json_type_parser_base<size_t> {
        inline static std::string_view::iterator parse(const std::string_view& _json, size_t& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
            std::string_view::iterator end = parse::first_special(start, _json.end());

            if (*end != ',' && *end != '}')
                return _json.end();

            std::string str(start, end);
            for (char& c : str)
                if (!std::isdigit(c))
                    return _json.end();
            _value = std::stoull(str);
            return end++;
        }

        inline static void stringify(size_t& _value, std::string& _out) {
            _out += std::to_string(_value);
        }

        static constexpr bool number_support = true;
    };

    template <>
    struct json_type_parser<float> : public json_type_parser_base<float> {
        inline static std::string_view::iterator parse(const std::string_view& _json, float& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
            std::string_view::iterator end = parse::first_special(start, _json.end());

            while (end != _json.end() && (*end == '.' || *end == '-' || *end == '+'))
                end = parse::first_special(end + 1, _json.end());

            if (*end != ',' && *end != '}')
                return _json.end();

            std::string str(start, end);
            for (char& c : str)
                if (!std::isdigit(c) && c != '.' && c != '-' && c != '+' && c != 'e' && c != 'E')
                    return _json.end();
            _value = std::stof(str);
            return end++;
        }

        inline static void stringify(float& _value, std::string& _out) {
            _out += std::to_string(_value);
        }

        static constexpr bool number_support = true;
    };

    template <>
    struct json_type_parser<std::string> : public json_type_parser_base<std::string> {
        inline static std::string_view::iterator parse(const std::string_view& _json, std::string& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
            std::string str = parse::extract_string(start, _json.end());
            if (str == "" && (*start != '"' || *(start + 1) != '"')) {
                return _json.end();
            }
            _value = str;
            return start + str.size() + 2;
        }

        inline static void stringify(std::string& _value, std::string& _out) {
            _out += '"' + _value + '"';
        }

        static constexpr bool string_support = true;
    };

    template <>
    struct json_type_parser<vector3<unsigned char>> : public json_type_parser_base<vector3<unsigned char>> {
        inline static std::string_view::iterator parse(const std::string_view& _json, vector3<unsigned char>& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
            std::string str = parse::extract_string(start, _json.end());
            if (str == "" && (*start != '"' || *(start + 1) != '"')) {
                return _json.end();
            }

            _value.x = utils::hex_byte_to_char({ start + 3, start + 5 });
            _value.y = utils::hex_byte_to_char({ start + 5, start + 7 });
            _value.z = utils::hex_byte_to_char({ start + 7, start + 9 });

            return start + str.size() + 2;
        }

        inline static void stringify(vector3<unsigned char>& _value, std::string& _out) {
            _out += '"' + std::format("0x{:02X}{:02X}{:02X}", _value.x, _value.y, _value.z) + '"';
        }

        static constexpr bool string_support = true;
    };

    template <>
    struct json_type_parser<bool> : public json_type_parser_base<bool> {
        inline static std::string_view::iterator parse(const std::string_view& _json, bool& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());

            if (start == _json.end())
                return _json.end();

            if (std::string_view(start, start + sizeof("true") - 1) == "true") {
                _value = true;
                return start + sizeof("true") - 1;
            } else if (std::string_view(start, start + sizeof("false") - 1) == "false") {
                _value = false;
                return start + sizeof("false") - 1;
            }

            return _json.end();
        }

        inline static void stringify(bool& _value, std::string& _out) {
            _out += _value ? "true" : "false";
        }

        static constexpr bool bool_support = true;
    };

    template <typename T>
        requires std::is_enum_v<T>
    struct json_type_parser<T> : public json_type_parser_base<T> {
        inline static std::string_view::iterator parse(const std::string_view& _json, T& _value) {
            std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
            std::string str = parse::extract_string(start, _json.end());
            if (str == "" && (*start != '"' || *(start + 1) != '"')) {
                return _json.end();
            }
            _value = meta::string_to_enum<T>(str);
            return start + str.size() + 2;
        }

        inline static void stringify(T& _value, std::string& _out) {
            _out += '"' + std::string(meta::enum_to_string(_value)) + '"';
        }

        static constexpr bool string_support = true;
    };
} // namespace mars::json