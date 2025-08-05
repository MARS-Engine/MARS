# imgui Struct Editor

imgui struct editor implementation can be [found here](../include/mars/imgui/struct_editor.hpp)

Our imgui struct editor is extremely simple to use and implement. Here is a usage example:

```c++
//this wil automatically render a ImGui::InputText
std::string name;
mars::imgui::struct_editor<std::string>::render(name /* value */, "Name" /* label */);
```

But our struct editor is much more powerfull than simply auto render a single type, it also supports structs. By default it uses `std::meta::nonstatic_data_members_of` to go through each data member in a struct and `mars::imgui::struct_editor<T>::render` on it.

To customize a type editor you simply specialize the `struct_editor` like this:

```c++
template <>
struct struct_editor<size_t> {
    static void render(size_t& _value, const std::string_view& _label) {
        ImGui::InputInt(_label.begin(), reinterpret_cast<int*>(&_value));
    }
};
```

Obviously you can edit it to show complex custom editors for your structs and this will automatically apply to said specialized type every time its found.

An example of a imgui editor see [struct_editor\<graphics::texture\>::render](../src/graphics/texture.cpp) where you can see an ImGui custom editor where it automatically renders textures and allows you to at runtime select a icon from a folder and load it directly into the texture slot in the struct