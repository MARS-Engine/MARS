#pragma once

#include <bitset>
#include <vector>

namespace mars {
    template <class T, size_t chuck_size>
        requires(chuck_size > 1)
    class sparse_vector {
        typedef std::pair<std::bitset<chuck_size>, T*> sparse_entry;
        std::vector<sparse_entry> m_data;
        std::vector<sparse_entry*> m_available_list;

        T* get_new_entry() {
            if (!m_available_list.size())
                m_available_list.push_back(&m_data.emplace_back(sparse_entry(std::bitset<chuck_size>(), new T[chuck_size])));

            sparse_entry* entry = m_available_list.at(0);

            for (size_t i = 0; i < chuck_size; ++i) {
                if (!entry->first.test(i)) {
                    entry->first.set(i);

                    if (entry->first.all())
                        m_available_list.erase(m_available_list.begin() + 0);
                    return entry->second + i;
                }
            }
            std::unreachable();
        }

      public:
        sparse_vector() = default;
        sparse_vector(const sparse_vector&) = delete;
        sparse_vector(sparse_vector&& _move) noexcept { *this = std::move(_move); }
        ~sparse_vector() { clear(); }

        T* request_entry() { return get_new_entry(); }

        void remove(T* _entry) {
            if (!m_data.size())
                return;

            sparse_entry* found = nullptr;
            for (auto& e : m_data) {
                if (e.second <= _entry && _entry < e.second + chuck_size) {
                    found = &e;
                    break;
                }
            }

            found->first.set(_entry - found->second, false);
            *_entry = T();
        }

        void clear() {
            for (sparse_entry& entry : m_data)
                delete[] entry.second;

            m_data.clear();
            m_available_list.clear();
        }

        sparse_vector& operator=(const sparse_vector& _copy) = delete;

        sparse_vector& operator=(sparse_vector&& _move) noexcept {
            if (this == &_move)
                return *this;

            swap(m_data, _move.m_data);
            swap(m_available_list, _move.m_available_list);

            return *this;
        }
    };
} // namespace mars