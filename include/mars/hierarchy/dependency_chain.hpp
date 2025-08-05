#pragma once

#include <mars/utility/iterator.hpp>
#include <mars/utility/pointer.hpp>
#include <stack>
#include <vector>

namespace mars {
    template <typename T>
    struct dependency_entry {
        pointer<T> entry;
        size_t dependents;
    };

    template <typename T>
    class dependency_chain {
      private:
        std::vector<dependency_entry<T>> m_entries;
        bool m_has_garbage = false;

      public:
        dependency_chain() = default;

        auto begin() const { return m_entries.begin(); }
        auto end() const { return m_entries.end(); }
        size_t size() const { return m_entries.size(); }

        void make(T* _ptr) {
            m_entries.push_back(dependency_entry{ pointer(_ptr), 0 });
        }

        void add(T* _parent, T* _ptr) {
            for (size_t i = 0; i < m_entries.size(); i++) {
                if (m_entries[i].entry == _parent) {
                    ++m_entries[i].dependents;
                    m_entries.insert(m_entries.begin() + i + m_entries[i].dependents, dependency_entry{ pointer(_ptr), 0 });
                    return;
                }
            }
        }

        [[nodiscard]]
        bool has_garbage() const { return m_has_garbage; }

        void mark_garbage(T* _ptr) {
            for (size_t i = 0; i < m_entries.size(); i++) {
                if (m_entries[i].entry == _ptr) {
                    m_has_garbage = true;
                    m_entries[i].entry.mark();
                    size_t dependents = m_entries[i].dependents;
                    iterator<dependency_entry<T>> position = m_entries.begin() + i;
                    while (dependents) {
                        dependents += m_entries[i + dependents].dependents;
                        --dependents;
                        ++position;
                        position->entry.mark();
                    }
                }
            }
        }

        bool is_marked(const T* _ptr) const {
            for (auto& entry : m_entries)
                if (entry.entry == _ptr)
                    return entry.entry.is_marked();
            return false;
        }

        void clear() {
            m_entries.clear();
        }
    };
} // namespace mars