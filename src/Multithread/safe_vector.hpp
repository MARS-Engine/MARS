#ifndef __SAFE__VECTOR__
#define __SAFE__VECTOR__

#include <vector>
#include <mutex>
#include <algorithm>
#include "Debug/debug.hpp"

using namespace std;

template<typename T> struct safe_vector {
private:
    mutable mutex _mtx;
    vector<T> _data;
public:


    void push_back(T _value) {
        lock_guard<mutex> l(_mtx);
        if (std::find(_data.begin(), _data.end(), _value) != _data.end())
            return debug::alert("attempted to add an already existing object");
        _data.push_back(_value);
    }

    typename vector<T>::const_iterator begin() const {
        lock_guard<mutex> l(_mtx);
        return _data.begin();
    }

    typename vector<T>::const_iterator end() const {
        lock_guard<mutex> l(_mtx);
        return _data.end();
    }

    T at(size_t _index) const {
        lock_guard<mutex> l(_mtx);
        return _data[_index];
    }

    T operator[](size_t _index) const {
        lock_guard<mutex> l(_mtx);
        return _data[_index];
    }

    bool empty() {
        lock_guard<mutex> l(_mtx);
        return _data.empty();
    }

    void erase(T _value) {
        lock_guard<mutex> l(_mtx);
        auto e = find(_data.begin(), _data.end(), _value);
        if (e == _data.end())
            return debug::alert("Attempted to delete non existing item");
        _data.erase(e);
    }

    size_t size() const {
        return _data.size();
    }

    void erase_at(size_t index) {
        lock_guard<mutex> l(_mtx);
        _data.erase(_data.begin() + index);
    }

    void clear() {
        lock_guard<mutex> l(_mtx);
        _data.clear();
    }
};

#endif