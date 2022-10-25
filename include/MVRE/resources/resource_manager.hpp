#ifndef __MVRE__RESOURCE__MANAGER__
#define __MVRE__RESOURCE__MANAGER__

#include "MVRE/debug/debug.hpp"
#include "MVRE/graphics/backend/base/base_types.hpp"
#include "MVRE/graphics/engine_instance.hpp"

#include <pl/safe_map.hpp>
#include <string>
#include <vector>

namespace mvre_resources {

    class resource_base {
    protected:
        mvre_graphics::engine_instance* instance = nullptr;
    public:
        inline void set_instance(mvre_graphics::engine_instance* _instance) { instance = _instance; }

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
        static std::map<mvre_graphics_base::MVRE_RESOURCE_TYPE, std::string> resources_locations;

        /***
         * load or get cached resource
         * @tparam T type of resource
         * @param _path path to resource
         * @param _resource reference to resource pointer
         * @return true if successfully loads or finds cached or false otherwise
         */
        template<typename T> static bool load_resource(const std::string& _path, T*& _resource, mvre_graphics::engine_instance* _instance) {
            static_assert(std::is_base_of<resource_base, T>::value, "invalid resource type, T must be derived from resource_base");

            auto temp_resource = get_cached_resource<T>(_path);

            if (temp_resource != nullptr) {
                _resource = (T*)resources[_path];
                return true;
            }

            temp_resource = new T();
            temp_resource->set_instance(_instance);
            if (!temp_resource->load_resource(_path)) {
                delete temp_resource;
                return false;
            }

            resources[_path] = temp_resource;
            _resource = temp_resource;
            return true;
        }

        /***
         * Read a file and store it to a line vector
         * @param _path path to file
         * @param data reference to the vector where it will be stored
         * @return returns true if it was successful
         */
        static bool read_file(const std::string& _path, std::vector<std::string>& data);

        static void clean();
    };
}

#endif