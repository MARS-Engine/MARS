#pragma once

#include <cstdint>
#include <format>
#include <meta>
#include <string_view>

namespace mars::hash {
    namespace detail {
        // note about this functions insde detail and hash logic is pure AI slop.
        // since it works™ and its supposed to be runtime only I dont really care about it for now but i really need to make something actually good in the future (assuming this isnt good enough as is)

        consteval std::string constexpr_int_to_string(int value) {
            std::array<char, 12> buffer{};
            auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
            std::size_t len = ptr - buffer.data();
            return std::string(buffer.data(), len);
        }

        consteval uint64_t fnv1a64(std::string_view s) {
            uint64_t h = 14695981039346656037ull;
            for (unsigned char c : s) {
                h ^= c;
                h *= 1099511628211ull;
            }
            return h;
        }

        consteval void hash_append(uint64_t& h, std::string_view s) {
            h ^= fnv1a64(s) + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
        }

        consteval void hash_entity(uint64_t& h, std::meta::info r) {
            using namespace std::meta;

            if (std::meta::has_identifier(r))
                hash_append(h, std::meta::identifier_of(r));
            else
                hash_append(h, std::meta::display_string_of(r));

            if (std::meta::is_namespace_member(r)) { //|| std::meta::has_parent(r)) { // has_parent is missing from current implementation
                auto p = parent_of(r);
                if (p != std::meta::info{})
                    hash_entity(h, p);
            } else {
                auto loc = source_location_of(r);
                hash_append(h, loc.file_name());
                hash_append(h, std::string_view(loc.function_name()));
                hash_append(h, constexpr_int_to_string(loc.line()));
                hash_append(h, constexpr_int_to_string(loc.column()));
            }
        }

        template <class T>
        consteval uint64_t type_fingerprint_ce() {
            uint64_t h = 0xcbf29ce484222325ull;
            hash_entity(h, std::meta::dealias(^^T));
            return h;
        }
    } // namespace detail

    // only use this for identifying types at runtime. UNDER NO CIRCUMSTANCES EVER STORE THIS VALUE IN ASSET DATA OR I WILL CURSE YOUR BLOODLINE FOR A THOUSAND YEARS
    template <class T>
    constexpr uint64_t type_fingerprint_v = detail::type_fingerprint_ce<T>();
} // namespace mars::hash