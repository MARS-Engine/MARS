#pragma once

#include <mars/container/sparse_array.hpp>
#include <mars/hierarchy/dependency_chain.hpp>
#include <ranges>
#include <stack>
#include <string_view>
#include <vector>

namespace mars::ui {

    struct html_block {
        std::string name;
        std::string id;
        std::map<std::string, std::string> attributes;
        std::string content;
    };

    class html_loader {
        mars::dependency_chain<html_block> widget_chain;
        std::vector<html_block> widget_data;

        struct html_load_helper {
            // keeps track of widget hierarchy
            mars::dependency_chain<html_block> helper_widget_chain;
            // actua data of widget
            mars::sparse_vector<html_block, 64> helper_widget_data;
        };

        template <bool Negate = false>
        inline auto first_special(const std::string_view::iterator& begin, const std::string_view::iterator& end) {
            return std::ranges::find_if(begin, end, [](char c) -> bool {
                return (!std::isalnum(static_cast<unsigned char>(c)) && !std::isspace(static_cast<unsigned char>(c))) ^ Negate;
            });
        }

        template <bool Negate = false>
        inline auto first_space(const std::string_view::iterator& begin, const std::string_view::iterator& end) {
            return std::ranges::find_if(begin, end, [](char c) -> bool {
                return std::isspace(static_cast<unsigned char>(c)) ^ Negate;
            });
        }

        std::string_view::iterator extract_attribute(html_block& _entry, const std::string_view& _view) {
            std::array<char, 3> valid_name_special_char = { '_', '-', '.' };

            std::string_view::iterator curr = _view.begin();
            while (*curr != '>' && curr != _view.end()) {
                std::string_view::iterator end = curr;
                do {
                    end++;
                    curr = first_space<true>(curr, _view.end());
                    end = first_special(end, _view.end());
                } while (std::ranges::contains(valid_name_special_char, *end));

                std::println("attribute name - {}", std::string_view(curr, end));
                if (*end != '=') {
                    std::println("invalid character '{}', '=' expected, skipping attribute", *end);
                    continue;
                }

                std::string name = std::string(curr, end);

                // skip =
                end++;
                curr = first_special(end, _view.end());

                if (curr == _view.end() || *curr != '"') {
                    std::println("invalid character '{}', '\"' expected, skipping attribute", *curr);
                    continue;
                }

                curr++;
                end = std::ranges::find(curr, _view.end(), '"');
                _entry.attributes[name] = std::string(curr, end);
                std::println("attribute value - {}", _entry.attributes[name]);
                curr = end + 1;
            }

            return curr;
        }

        std::string_view::iterator extract_entries(html_block& _entry, const std::string_view& _view) {
            auto tag_begin = first_space<true>(_view.begin(), _view.end());
            auto tag_end = first_space(tag_begin, _view.end());

            _entry.name = std::string(tag_begin, tag_end);
            std::println("tag - {}", std::string_view(tag_begin, tag_end));

            tag_end++;

            return extract_attribute(_entry, std::string_view(tag_end, _view.end())) + 1;
        }

        std::string_view::iterator find_next_tag(html_load_helper& _helper, const std::string_view& _view, html_block* _parent = nullptr) {
            html_block& entry = *_helper.helper_widget_data.request_entry();

            std::string_view::iterator start_pos = _view.begin();
            bool b_tag_found = false;

            while (!b_tag_found && start_pos != _view.end()) {
                start_pos = first_space<true>(start_pos, _view.end());
                if (*start_pos != '<')
                    std::println("invalid character found");
                else
                    b_tag_found = true;
                start_pos++;
            }

            if (start_pos == _view.end()) {
                _helper.helper_widget_data.remove(&entry);
                return start_pos;
            }

            std::string_view::iterator entry_end = extract_entries(entry, std::string_view(start_pos, _view.end()));
            // extract content

            std::string_view::iterator curr = entry_end;

            if (_parent)
                _helper.helper_widget_chain.add(_parent, &entry);
            else
                _helper.helper_widget_chain.make(&entry);

            do {
                std::string_view::iterator start = curr;
                curr = std::ranges::find(curr, _view.end(), '<');
                entry.content += std::string(start + 1, curr);

                if (*(curr + 1) != '/')
                    curr = find_next_tag(_helper, std::string_view(curr, _view.end()), &entry);
                else if (std::string_view(curr + 2, curr + 2 + entry.name.size()) != entry.name) {
                    std::println("invalid block end for {}", entry.name);
                } else
                    break;
            } while (curr != _view.end());

            std::println("content - {}", entry.content);
            std::string_view::iterator res = curr + 2 + entry.name.size();
            return res;
        }

      public:
        const mars::dependency_chain<html_block>& get_widget_chain() const { return widget_chain; }

        void load_file(const std::string& _file) {
            struct html_dependency_helper {
                html_block* parent;
                size_t children_left;
            };

            html_load_helper helper;
            std::string data = mars::io::read_file(_file);
            std::string_view processor = data;
            std::string_view::iterator pos = processor.begin();

            while (pos != processor.end()) {
                pos = find_next_tag(helper, std::string_view(pos, processor.end()));
                if (pos != processor.end())
                    pos++;
            }

            widget_data.reserve(helper.helper_widget_chain.size());

            std::stack<html_dependency_helper> parent_stack;

            for (const dependency_entry<html_block>& ptr : helper.helper_widget_chain) {
                html_block& new_block = widget_data.emplace_back(*ptr.entry);

                if (parent_stack.size()) {
                    widget_chain.add(parent_stack.top().parent, &new_block);
                    parent_stack.top().children_left--;
                } else {
                    widget_chain.make(&new_block);
                }

                if (ptr.dependents) {
                    parent_stack.push({ &widget_data[widget_data.size() - 1], ptr.dependents });
                    continue;
                }

                while (parent_stack.size() && parent_stack.top().children_left == 0)
                    parent_stack.pop();
            }
        }
    };
} // namespace mars::ui