#ifndef __MVRE__RESOURCE__MANAGER__
#define __MVRE__RESOURCE__MANAGER__

#include <MVRE/debug/debug.hpp>
#include <MVRE/graphics/backend/template/graphics_component.hpp>
#include <MVRE/graphics/backend/template/graphics_types.hpp>
#include <MVRE/graphics/graphics_instance.hpp>
#include <pl/safe_map.hpp>
#include <string>
#include <vector>
#include <sys/stat.h>

namespace mvre_resources {

    class resource_base {
    public:
        inline virtual bool load_resource(const std::string& _path) { return false; }
        inline virtual void clean() { }
    };

    class resource_manager {
    private:
        static pl::safe_map<std::string, resource_base*> resources;

        /***
        * Get a pointer to a resource
        * @tparam T the type of resource exp.: shader
        * @param _path the path to the file
        * @return pointer to resource or nullptr
        */
        template<typename T> static T* get_cached_resource(const std::string& _path) {
            if (!resources.has(_path))
                return nullptr;
            return dynamic_cast<T*>(resources[_path]);
        }
    public:
        static std::map<mvre_graphics::MVRE_RESOURCE_TYPE, std::string> resources_locations;

        /***
        * load or get cached graphical resource, exp: textures, shaders
        * @tparam T type of resource, must have resource_base and graphics_base
        * @param _path path to resource
        * @param _resource reference to resource pointer
        * @return true if successfully loads or finds cached or false otherwise
        */
        template<typename T> static bool load_graphical_resource(const std::string& _path, T*& _resource, mvre_graphics::graphics_instance* _instance) {
            static_assert(std::is_base_of<resource_base, T>::value, "invalid resource type, T must be derived from resource_base and graphics_base");
            static_assert(std::is_base_of<mvre_graphics::graphics_component, T>::value, "invalid resource type, T must be derived from backend_base");

            auto temp_resource = get_cached_resource<T>(_path);

            if (temp_resource != nullptr) {
                _resource = (T*)resources[_path];
                return true;
            }

            temp_resource = _instance->instance<T>();
            if (!temp_resource->load_resource(_path)) {
                delete temp_resource;
                return false;
            }

            resources[_path] = temp_resource;
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
        template<typename T> static bool load_resource(const std::string& _path, T*& _resource, mvre_graphics::graphics_instance* _instance) {
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
        template<typename T> static bool load_resource(const std::string& _path, T*& _resource) {
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

        /***
         *
         */
        static inline bool file_exists(const std::string& _file) {
            struct stat buffer{};
            return (stat(_file.c_str(), &buffer) == 0);
        }

        /***
         * Read a file and store it to a line vector
         * @param _path path to file
         * @param data reference to the vector where it will be stored
         * @return returns true if it was successful
         */
        static bool read_file(const std::string& _path, std::vector<std::string>& data);

        /***
        * Read a binary file and store it to a line vector
        * @param _path path to file
        * @param data reference to the vector where it will be stored
        * @return returns true if it was successful
        */
        static bool read_binary(const std::string& _path, std::vector<char>& data);

        static std::string find_path(const std::string& _file, mvre_graphics::MVRE_RESOURCE_TYPE _type);

        static void clean();
    };
}

#endif