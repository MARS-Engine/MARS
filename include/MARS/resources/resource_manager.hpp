#ifndef MARS_RESOURCE_MANAGER_
#define MARS_RESOURCE_MANAGER_

#include <MARS/debug/debug.hpp>
#include <MARS/graphics/backend/template/graphics_types.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include <pl/safe_map.hpp>
#include <string>
#include <vector>
#include <sys/stat.h>

namespace mars_graphics {
    class graphics_component;
}

namespace mars_resources {

    class resource_base {
    public:
        inline virtual bool load_resource(const std::string& _path) { return false; }
        inline virtual void clean() { }
    };

    class resource_manager {
    private:
        pl::safe_map<std::string, resource_base*> resources;
        std::map<mars_graphics::MARS_RESOURCE_TYPE, std::string> resources_locations = {
                { mars_graphics::MARS_RESOURCE_TYPE_SHADER, "engine/assets/shaders/" },
                { mars_graphics::MARS_RESOURCE_TYPE_TEXTURE, "engine/assets/textures/" },
                { mars_graphics::MARS_RESOURCE_TYPE_MATERIAL, "engine/assets/materials/" },
                { mars_graphics::MARS_RESOURCE_TYPE_RENDERER, "engine/assets/renderer/" }
        };

        /***
        * Get a pointer to a resource
        * @tparam T the type of resource exp.: shader
        * @param _path the path to the file
        * @return pointer to resource or nullptr
        */
        template<typename T> T* get_cached_resource(const std::string& _path) {
            return resources.contains(_path) ? dynamic_cast<T*>(resources[_path]) : nullptr;
        }
    public:
        [[nodiscard]] std::string get_location(mars_graphics::MARS_RESOURCE_TYPE _type) const { return resources_locations.at(_type); }

        /***
        * load or get cached graphical resource, exp: textures, shaders
        * @tparam T type of resource, must have resource_base and graphics_base
        * @param _path path to resource
        * @param _resource reference to resource pointer
        * @return true if successfully loads or finds cached or false otherwise
        */
        template<typename T> bool load_graphical_resource(const std::string& _path, T*& _resource, mars_graphics::graphics_engine* _instance) {
            static_assert(std::is_base_of<resource_base, T>::value, "invalid resource type, T must be derived from resource_base and graphics_base");
            static_assert(std::is_base_of<mars_graphics::graphics_component, T>::value, "invalid resource type, T must be derived from backend_base");

            resources.lock();

            auto temp_resource = get_cached_resource<T>(_path);

            if (temp_resource != nullptr) {
                _resource = (T*)resources[_path];
                resources.unlock();
                return true;
            }

            temp_resource = _instance->create<T>();
            if (!temp_resource->load_resource(_path)) {
                delete temp_resource;
                resources.unlock();
                return false;
            }

            resources[_path] = temp_resource;
            resources.unlock();

            _resource = temp_resource;
            return true;
        }

        /***
        * load or get cached graphical resource, exp: textures, shaders
        * @tparam T type of resource, must have resource_base and graphics_base
        * @param _path path to resource
        * @param _resource reference to resource pointer
        * @return true if successfully loads or finds cached or false otherwise
        */
        template<typename T> bool load_resource(const std::string& _path, T*& _resource, mars_graphics::graphics_engine* _instance) {
            static_assert(std::is_base_of<resource_base, T>::value, "invalid resource type, T must be derived from resource_base and graphics_base");

            auto temp_resource = get_cached_resource<T>(_path);

            if (temp_resource != nullptr) {
                _resource = (T*)resources[_path];
                return true;
            }

            temp_resource = new T(_instance);
            if (!temp_resource->load_resource(_path)) {
                delete temp_resource;
                return false;
            }

            resources[_path] = temp_resource;
            _resource = temp_resource;
            return true;
        }

        /***
         * load or get cached resource
         * @tparam T type of resource, must have resource_base and graphics_base
         * @param _path path to resource
         * @param _resource reference to resource pointer
         * @return true if successfully loads or finds cached or false otherwise
         */
        template<typename T> bool load_resource(const std::string& _path, T*& _resource) {
            static_assert(std::is_base_of<resource_base, T>::value, "invalid resource type, T must be derived from resource_base and resource_base");

            auto temp_resource = get_cached_resource<T>(_path);

            if (temp_resource != nullptr) {
                _resource = (T*)resources[_path];
                return true;
            }

            temp_resource = new T();
            if (!temp_resource->load_resource(_path)) {
                delete temp_resource;
                return false;
            }

            resources[_path] = temp_resource;
            _resource = temp_resource;
            return true;
        }


        static inline bool file_exists(const std::string& _file) {
            struct stat buffer{};
            return (stat(_file.c_str(), &buffer) == 0);
        }

        static bool read_file(const std::string& _path, std::vector<std::string>& data);

        static bool read_binary(const std::string& _path, std::vector<char>& data);

        std::string find_path(const std::string& _file, mars_graphics::MARS_RESOURCE_TYPE _type, const std::string& _path_suffix = "");

        void clean();
    };
}

#endif