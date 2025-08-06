# Event System

Event system implementation can be [found here.](../include/mars/event/event.hpp)

Currently, the system supports only simple events, meaning you can only pass static or non-member functions and the context object can't move, this is not due to c++ constraints it's just that I don't need more right now, in the future we should add support for more.

This event system was carefully designed to avoid undefined behavior, particularly the ABI issues that can arise from unsafe `reinterpret_cast` usage on non-Intel/AMD platforms.

## Usage

Here is an example of a button event

```c++
struct button_events {
    void on_click(struct button&, const std::string&);
};

struct button : public mars::event<button_events> {
    std::string text;

    void click() {
        broadcast<&button_events::on_click>(*this, text);
    };
};

struct ui_context {
    button btn;
};

void on_button_clicked(button& _btn, const std::string& _str, ui_context& _context) {
    std::println("{} | {} | {}", _btn.text, _str, _context.btn.text);
}

int main() {
    ui_context ui;
    ui.btn.text = "i am a button";
    ui.btn.listen<&button_events::on_click, &on_button_clicked>(ui);
    ui.btn.click(); // this will triggerd the event
    ui.btn.stop_listening<&button_events::on_click, &on_button_clicked>(ui);
    ui.btn.click(); // event will no longer be triggerd
}

```