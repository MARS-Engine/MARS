#pragma once

#include <mars/debug/logger.hpp>
#include <mars/hash/meta.hpp>
#include <unordered_map>

namespace mars {
    struct entity {
        uint32_t index;
        uint32_t generation;
    };

    template <typename T>
    struct entity_component_storage {
      private:
        inline static mars::log_channel log_entity_channel = "game/entity_component_storage/";

      public:
        std::vector<uint32_t> entities;
        std::vector<T> data;
        std::unordered_map<std::uint32_t, std::size_t> lookup;

        bool has(const entity& _entity) { return lookup.contains(_entity.index); }

        // we recommend using this in case the storage changes in the future
        bool has_index(uint32_t _index) { return lookup.contains(_index); }

        T& get(const entity& _entity) { return const_cast<T&>(const_cast<const entity_component_storage<T>*>(this)->get(_entity)); }
        const T& get(const entity& _entity) const { return get(_entity.index); }

        T& get(size_t _index) { return const_cast<T&>(const_cast<const entity_component_storage<T>*>(this)->get(_index)); }
        const T& get(size_t _index) const { return data.at(lookup.at(_index)); }

        size_t size() const { return entities.size(); }
        uint32_t entity_at(std::size_t _index) const { return entities[_index]; }

        template <typename... Args>
        void emplace_back(const entity& _entity, Args&&... _args) {
            if constexpr (mars::environment::is_debug) {
                constexpr auto name_sv = std::meta::has_identifier(^^T) ? std::meta::identifier_of(^^T) : std::meta::display_string_of(^^T);
                mars::logger::assert_if(lookup.contains(_entity.index), log_entity_channel, "attempted to add entity that already exists to entity_component_storage<{}>", name_sv);
            }

            size_t pos = entities.size();
            entities.push_back(_entity.index);
            data.emplace_back(_args...);
            lookup.emplace(_entity.index, pos);
        }

        void remove(const entity& _entity) {
            auto it = lookup.find(_entity.index);

            if (it == lookup.end())
                return;

            size_t pos = it->second;
            size_t last = entities.size() - 1;
            if (pos != last) {
                uint32_t moved_index = entities[last];
                entities[pos] = moved_index;
                data[pos] = std::move(data[last]);
                lookup[moved_index] = pos;
            }
            entities.pop_back();
            data.pop_back();
            lookup.erase(it);
        }
    };

    class engine {
      private:
        struct entity_slot {
            uint32_t generation;
            bool alive;
        };

        struct component_entry {
            void* ptr;
            void (*deleter)(void*);
            void (*remove_entity)(void*, const entity&);
        };

        std::unordered_map<size_t, component_entry> m_components;
        std::vector<entity_slot> entities;
        std::vector<uint32_t> free_slots;

        entity entity_create_internal() {
            uint32_t idx;
            if (!free_slots.empty()) {
                idx = free_slots.back();
                free_slots.pop_back();
            } else {
                idx = static_cast<uint32_t>(entities.size());
                entities.emplace_back();
            }
            entities[idx].alive = true;
            return { idx, entities[idx].generation };
        }

      public:
        ~engine() {
            for (auto& pair : m_components)
                pair.second.deleter(pair.second.ptr);
            m_components.clear();
        }

        bool is_alive(const entity& _entity) const {
            return _entity.index < entities.size() && entities[_entity.index].alive && entities[_entity.index].generation == _entity.generation;
        }

        template <typename T>
        entity request_entity(const T& _entity = T()) {
            entity new_entity = entity_create_internal();

            constexpr std::meta::access_context ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                using C = [:std::meta::type_of(mem):];
                constexpr size_t type_id = mars::hash::type_fingerprint_v<C>;

                if (!m_components.contains(type_id))
                    m_components[type_id] = {
                        new entity_component_storage<C>(),
                        [](void* _ptr) { delete static_cast<entity_component_storage<C>*>(_ptr); },
                        [](void* _ptr, const entity& _dead_entity) { static_cast<entity_component_storage<C>*>(_ptr)->remove(_dead_entity); }
                    };

                entity_component_storage<C>& component = *static_cast<entity_component_storage<C>*>(m_components[type_id].ptr);
                component.emplace_back(new_entity, _entity.[:mem:]);
            }

            return new_entity;
        }

        void entity_destroy(const entity& _entity) {
            if (!is_alive(_entity))
                return;

            auto& entity = entities[_entity.index];
            entity.alive = false;
            ++entity.generation;
            free_slots.push_back(_entity.index);

            for (auto& pair : m_components)
                pair.second.remove_entity(pair.second.ptr, _entity);
        }

        template <typename... Args>
        void process(void (*_ptr)(const entity&, Args...)) {
            static_assert(sizeof...(Args) != 0, "At least one argument required");
            size_t smallest = -1;
            std::vector<uint32_t>* small_comp_entities = nullptr;
            constexpr std::meta::access_context ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::template_arguments_of(^^process<Args...>))) {
                using C = [:std::meta::remove_cvref(mem):];
                constexpr size_t type_id = mars::hash::type_fingerprint_v<C>;
                entity_component_storage<C>& component = *static_cast<entity_component_storage<C>*>(m_components[type_id].ptr);
                if (component.size() < smallest) {
                    smallest = component.size();
                    small_comp_entities = &component.entities;
                }
            }

            void* storage_ptrs[sizeof...(Args)];
            size_t storage_ptr;
            for (uint32_t index : *small_comp_entities) {
                storage_ptr = 0;
                template for (constexpr auto mem : std::define_static_array(std::meta::template_arguments_of(^^process<Args...>))) {
                    using C = [:std::meta::remove_cvref(mem):];
                    constexpr size_t type_id = mars::hash::type_fingerprint_v<C>;
                    entity_component_storage<C>& component = *static_cast<entity_component_storage<C>*>(m_components[type_id].ptr);
                    if (!component.has_index(index))
                        continue;
                    storage_ptrs[storage_ptr++] = &component.get(index);
                }

                entity e{ index, entities[index].generation };

                [&]<std::size_t... i>(std::index_sequence<i...>) {
                    _ptr(e, *static_cast<std::remove_cvref_t<Args>*>(storage_ptrs[i])...);
                }(std::make_index_sequence<sizeof...(Args)>{});
            }
        }
    };
} // namespace mars