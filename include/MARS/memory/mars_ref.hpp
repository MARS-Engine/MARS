#ifndef MARS_REF_
#define MARS_REF_

#include <memory>


template<typename T> class mars_ref {
private:
    std::weak_ptr<T> m_data;
public:
    T* ptr() const { return m_data.lock().get(); }
    const std::weak_ptr<T>& get() const { return m_data; }

    mars_ref() = default;

    explicit mars_ref(const std::weak_ptr<T>& _ptr) { m_data = _ptr; }

    template<typename C> mars_ref(const mars_ref<C>& _cast) {
        m_data = _cast.get();
    }

    template<typename C> mars_ref<C> cast_static() const {
        if (auto cast = m_data.lock())
            return mars_ref<C>(std::static_pointer_cast<C>(cast));
        return mars_ref<C>();
    }

    template<typename C> mars_ref<C> cast_dynamic() const {
        if (auto cast = m_data.lock())
            return mars_ref<C>(std::dynamic_pointer_cast<C>(cast));
        return mars_ref<C>();
    }

    [[nodiscard]] bool is_alive() const noexcept {
        return m_data.lock().get() != nullptr;
    }

    T* operator->() const {
        return m_data.lock().get();
    }

    bool operator<(const mars_ref& _other) const {
        return m_data.lock() < _other.get().lock();
    }

    bool operator==(const mars_ref& _other) const {
        return m_data.lock() == _other.get().lock();
    }
};

#endif