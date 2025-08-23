#pragma once

#include <mars/hash/meta.hpp>

namespace mars {

    // friendship token, while you hold this the associated resource will be kept alive
    struct resource_token {
      private:
        size_t* m_counter;

        explicit resource_token(size_t* _counter) : m_counter(_counter) {}

        friend class resource_manager;

      public:
        resource_token() = delete;
        ~resource_token() { m_counter--; }
        resource_token(const resource_token&) = delete;
        resource_token(resource_token&&) = default;
        resource_token& operator=(const resource_token&) = delete;
        resource_token& operator=(resource_token&&) = default;
    };

    class resource_manager {
      private:
      public:
        template <typename T>
        resource_token load_resource(const std::string_view& _path) {
            resource_token counter = resource_token(new size_t());

            return counter;
        }
    };
} // namespace mars