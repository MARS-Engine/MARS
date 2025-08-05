# JSON

json parser implementation can be [found here](../include/mars/parser/json/json.hpp)

Our json parser/stringify takes full use of static reflection to automate as much as possible.

Here is an example:

```c++
#include <mars/parser/json/json.hpp>

//parse value from json
std::string json_string;
T value;

// updates value with the contents of json_string
mars::json::json_type_parser<std::vector<county>>::parse(json_string, value);

//stringify value and saves it to json_string
mars::json::json_type_parser<std::vector<county>>::stringify(value, json_string);
```

For structs and classes, it uses `std::meta::nonstatic_data_members_of` to automatically loop through all data members and stringify/parse them.

The way the parser knows how to parse or stringify a json is by using specializations of `mars::json::json_type_parser` if a member of a struct is not specialized it will be ignored.

## How to specialize `mars::json::json_type_parser`

this is a barebones example of what specialization looks like, you must explicitly inherit from `mars::json::json_type_parser_base<T>` and declare `parse` and `stringify`:

```c++
template <>
struct mars::json::json_type_parser<float> : public mars::json::json_type_parser_base<float> {
    inline static std::string_view::iterator parse(const std::string_view& _json, float& _value) {
        return _json.start() + sizeof("value_here") - 1;
    }

    inline static void stringify(float& _value, std::string& _out) {
        _out += "value_here";
    }
};
```

You can also declare the following bools to tell the parser if a certain type is supported:
```c++
static constexpr bool array_support = false; // if a value starts with '[' its an array
static constexpr bool string_support = false; // if a value starts with '"' its an string
static constexpr bool bool_support = false; // if a value starts with 't' or 'f' its an bool
static constexpr bool number_support = false; // if a value starts with -,+ or digit its an number
static constexpr bool struct_support = false; // if a value starts with '{' its an struct
```

here is a full example in action, in this case we are telling the parser to treat a vector of 3 chars as a hex when parsing/stringify

```c++
template <>
struct json_type_parser<vector3<unsigned char>> : public json_type_parser_base<vector3<unsigned char>> {
    inline static std::string_view::iterator parse(const std::string_view& _json, vector3<unsigned char>& _value) {
        std::string_view::iterator start = parse::first_space<false>(_json.begin(), _json.end());
        std::string str = parse::extract_string(start, _json.end());

        if (str == "" && (*start != '"' || *(start + 1) != '"'))
            return _json.end();

        _value.x = utils::hex_byte_to_char({ start + 3, start + 5 });
        _value.y = utils::hex_byte_to_char({ start + 5, start + 7 });
        _value.z = utils::hex_byte_to_char({ start + 7, start + 9 });

        return start + str.size() + 2;
    }

    inline static void stringify(vector3<unsigned char>& _value, std::string& _out) {
        _out += '"' + std::format("0x{:02X}{:02X}{:02X}", _value.x, _value.y, _value.z) + '"';
    }

    static constexpr bool string_support = true; //we print it as a string but could also be a number since it always starts with 0
};
```

## Arrays

By default the engine already takes care of the `std::vector`. You can check `json_type_parser<std::vector<T>>` to see the default implementation in action. For any other array type you'll need to manually specialize the parser for that array type, this should be the bare bones needed to do it:

```c++
template <typename T>
struct json_type_parser<array_type<T>> : public json_type_parser_base<array_type<T>> {
    inline static std::string_view::iterator parse(const std::string_view& _json, array_type<T>& _value) {
        std::string_view::iterator current = parse::first_space<false>(_json.begin(), _json.end());
        if (*current != '[')
            return _json.end();
        current++;

        while (current != _json.end() && *current != ']') {
            // do parsing here
            else if (*current != ']')
                return _json.end();
        }
        return current;
    }

    inline static void stringify(array_type<T>& _value, std::string& _out) {
        _out += "[ ";
        
        //this should be the same even for every array type        
        for (T& entry : _value) {
            json_type_parser<T>::stringify(entry, _out);
            _out += ",";
        }

        _out[_out.size() - 1] = ']';
    }
    static constexpr bool array_support = true;
};
```