#ifndef __SAFE__VECTOR__
#define __SAFE__VECTOR__

#include <vector>
#include <mutex>
#include <algorithm>
#include "Debug/debug.hpp"



template<typename T> struct safe_vector {
private:
    mutable std::mutex _mtx;
    std::vector<T> _data;
public:


    void push_back(T _value) {
        std::lock_guard<std::mutex> l(_mtx);
        if (std::find(_data.begin(), _data.end(), _value) != _data.end())
            return debug::alert("attempted to add an already existing object");
        _data.push_back(_value);
    }

    typename std::vector<T>::const_iterator begin() const {
        std::lock_guard<std::mutex> l(_mtx);
        return _data.begin();
    }

    typename std::vector<T>::const_iterator end() const {
        std::lock_guard<std::mutex> l(_mtx);
        return _data.end();
    }

    T at(size_t _index) const {
        std::lock_guard<std::mutex> l(_mtx);
        return _data[_index];
    }

    T operator[](size_t _index) const {
        std::lock_guard<std::mutex> l(_mtx);
        return _data[_index];
    }

    bool empty() {
        std::lock_guard<std::mutex> l(_mtx);
        return _data.empty();
    }

    void erase(T _value) {
        std::lock_guard<std::mutex> l(_mtx);
        auto e = find(_data.begin(), _data.end(), _value);
        if (e == _data.end())
            return debug::alert("Attempted to delete non existing item");
        _data.erase(e);
    }

    size_t size() const {
        return _data.size();
    }

    void erase_at(size_t index) {
        std::lock_guard<std::mutex> l(_mtx);
        _data.erase(_data.begin() + index);
    }

    void clear() {
        std::lock_guard<std::mutex> l(_mtx);
        _data.clear();
    }
};

#endif