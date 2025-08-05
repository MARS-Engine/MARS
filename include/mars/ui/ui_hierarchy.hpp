#pragma once

#include "html_loader.hpp"
#include "mars/utility/pointer.hpp"
#include "widget_flex.hpp"

#include <SDL3/SDL_init.h>
#include <asm-generic/errno.h>
#include <mars/container/sparse_array.hpp>
#include <mars/hierarchy/dependency_chain.hpp>
#include <mars/math/vector2.hpp>
#include <mars/math/vector4.hpp>
#include <mars/text/font.hpp>
#include <stack>

namespace mars {
    namespace ui {

        class ui_hierarchy {
          private:
            // keeps track of widget hierarchy
            dependency_chain<widget_flex> widget_chain;
            // actua data of widget
            sparse_vector<widget_flex, 64> widget_data;

          public:
            auto begin() const { return widget_chain.begin(); }
            auto end() const { return widget_chain.end(); }

            widget_flex* request_widget(widget_flex* _parent = nullptr) {
                widget_flex* result = widget_data.request_entry();

                if (_parent)
                    widget_chain.add(_parent, result);
                else
                    widget_chain.make(result);

                return result;
            }

            void process_data(mars::ui_instance_data* _data, mars::vector2<float> _size) {
                struct parent_entry {
                    const mars::dependency_entry<mars::ui::widget_flex>* ptr;
                    mars::vector4<float> render_rect;
                    mars::vector2<float> offset;
                    size_t children_left;
                    size_t children;
                };

                std::stack<parent_entry> parents;
                mars::ui::widget_flex window_flex{};
                mars::dependency_entry<mars::ui::widget_flex> window_flex_entry{ &window_flex, 1 };
                parents.emplace(parent_entry{ &window_flex_entry, { { 0, 0 }, _size }, { 0, 0 }, 1, 1 });

                size_t i = 0;

                for (const mars::dependency_entry<mars::ui::widget_flex>& flex_entry : widget_chain) {
                    vector4<float> render_rect = parents.top().ptr->entry->get_child_rect(parents.top().render_rect, parents.top().children);
                    flex_entry.entry->apply_render_changes(render_rect);

                    _data[i] = mars::ui_instance_data{
                        .position = { render_rect.xy() + parents.top().offset, 100.0f - i * 0.01f },
                        .size = render_rect.zw(),
                        .uv_rect = {},
                        .color = flex_entry.entry->colour,
                        .radius = { 10.0f },
                    };

                    flex_entry.entry->apply_offset(*parents.top().ptr->entry, parents.top().offset, render_rect);

                    if (!(--parents.top().children_left) && flex_entry.dependents == 0)
                        parents.pop();

                    if (flex_entry.dependents) {
                        parents.emplace(parent_entry{ &flex_entry, render_rect, { 0, 0 }, flex_entry.dependents, flex_entry.dependents });
                    }

                    i++;

                    while (parents.size() && !(parents.top().children_left) && flex_entry.dependents == 0)
                        parents.pop();
                }
            }
        };

        struct widget_event {
            void (*on_click)(widget_flex&);
        };

        struct widget_annotation {
        };

        void spawn_widget(ui_hierarchy& _hierarchy, html_loader& _loader) {
            std::stack<mars::dependency_entry<widget_flex>> parents;

            for (const mars::dependency_entry<html_block>& entry : _loader.get_widget_chain()) {
                widget_flex* new_widget = _hierarchy.request_widget(parents.size() ? parents.top().entry.get() : nullptr);
                new_widget->content = entry.entry->content;

                if (parents.size()) {
                    parents.top().dependents--;
                }

                if (entry.dependents) {
                    parents.push({ pointer<widget_flex>(new_widget), entry.dependents });
                }

                while (parents.size() && parents.top().dependents == 0) {
                    parents.pop();
                }
            }
        }

        template <typename T>
        void bind_widget() {
            constexpr std::optional<widget_annotation> annoation = mars::meta::get_annotation<widget_annotation>(^^T);

            if constexpr (!annoation.has_value())
                static_assert(0, "pipeline draw call missing draw_propery");
        }
    } // namespace ui

    namespace prop {
        static constexpr mars::ui::widget_annotation widget(const char*) {
            return {};
        }

        template <auto T>
        static constexpr mars::ui::widget_annotation widget_event() {
            return {};
        }

    } // namespace prop
} // namespace mars