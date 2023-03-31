#ifndef MARS_REF_
#define MARS_REF_

#include <memory>

namespace mars_mem {

    template<typename T> class mars_ref {
    private:
        std::weak_ptr<T> m_data;
    public:
        std::weak_ptr<T> get() const { return m_data; }

        mars_ref() = default;

        explicit mars_ref(const std::shared_ptr<T>& _ref) {
            m_data = _ref;
        }

        template<typename C> explicit mars_ref(const mars_ref<C>& _cast) {
            m_data = _cast.get();
        }

        template<typename C> mars_ref<C> cast_static() {
            if (auto cast = m_data.lock())
                return mars_ref<C>(std::static_pointer_cast<C>(cast));
            return mars_ref<C>();
        }

        template<typename C> mars_ref<C> cast_dynamic() {
            if (auto cast = m_data.lock())
                return mars_ref<C>(std::dynamic_pointer_cast<C>(cast));
            return mars_ref<C>();
        }

        bool is_alive() {
            return m_data.expired();
        }

        T* operator->() {
            return m_data.lock().get();
        }
    };
}

#endif