# ImGui Struct Editor

ImGui struct editor implementation can be [found here](../include/mars/imgui/struct_editor.hpp)

Our ImGui struct editor is extremely simple to use and implement. Here is a usage example:

```c++
//this will automatically render a ImGui::InputText
std::string name;
mars::imgui::struct_editor<std::string> string_editor(name);
string_editor.render("Name" /* label */);
```

But our struct editor is much more powerful than simply auto render a single type, it also supports structs. By default it uses `std::meta::nonstatic_data_members_of` to go through each data member in a struct and calls `mars::imgui::struct_editor<T>::render` on it.

To customize a type editor you simply specialize the `struct_editor` like this:

```c++
template <>
struct struct_editor<size_t> : public struct_editor_base<size_t> {
    using struct_editor_base<size_t>::struct_editor_base;

    void render(const std::string_view& _label) {
        if (this->ref) // ptr to data being edited, its stored in struct_editor_base
            ImGui::InputInt(_label.begin(), reinterpret_cast<int*>(ref));
    }
};
```

`struct_editor_base` is optional, you don’t have to include it but it does help making cleaner code as it provides reference storage and common utilities to reduce boilerplate when writing custom editors.

Obviously you can edit it to show complex custom editors for your structs and this will automatically apply to said specialized type every time its found.

For an example of a custom ImGui editor, see [struct_editor\<graphics::texture\>::render](../include/mars/imgui/texture.hpp) where you can see an ImGui custom editor where it automatically renders textures and allows you to at runtime select a icon from a folder and load it directly into the texture slot in the struct