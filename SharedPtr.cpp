#include <atomic>
#include <iostream>
#include <utility>

template<typename T> 
class SharedPtr {
public:
    explicit SharedPtr(T* data = nullptr) : _data(data), _ref(nullptr) {
        if (_data) {
            try {
                _ref = new std::atomic<int>(1);
            } catch (...) {
                delete _data;
                throw;
            }
        }
    }

    ~SharedPtr() {
        release();
    } 

    // 这里不需要考虑 this
    SharedPtr(const SharedPtr& other) : _data(other._data), _ref(other._ref) {
        if (_ref) {
            _ref->fetch_add(1);
        }
    }

    SharedPtr(SharedPtr&& other) noexcept : _data(other._data), _ref(other._ref) {
        other._data = nullptr;
        other._ref = nullptr;
    }

    SharedPtr& operator=(const SharedPtr& other)  {
        if (this != &other) {
            release();
            _data = other._data;
            _ref = other._ref;
            if (_ref) {
                _ref->fetch_add(1);
            }
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            _data = other._data;
            _ref = other._ref;
            other._data = nullptr;
            other._ref = nullptr;
        }
        return *this;
    }

    // const
    T* operator->() const {
        return _data;
    }

    // operator*
    T& operator*() const {
        return *_data;
    }

    T* get() const {
        return _data;
    }

    //explicit
    explicit operator bool() const {
        return _data != nullptr;
    }

    void reset(T* data = nullptr) {
        SharedPtr temp(data);
        swap(temp);
    }

    int count() const {
        return _ref ? _ref->load() : 0;
    }

    void swap(SharedPtr& other) noexcept {
        std::swap(_data, other._data);
        std::swap(_ref, other._ref);
    }

    friend void swap(SharedPtr& lhs, SharedPtr& rhs) noexcept {
        lhs.swap(rhs);
    }


private:
    void release() {
        if (!_ref) {
            _data = nullptr;
            return;
        }
        if (_ref->fetch_sub(1) == 1) {
            delete _data;
            delete _ref;
        }
        _data = nullptr;
        _ref = nullptr;
    }

    T* _data = nullptr;
    std::atomic<int>* _ref = nullptr;
};

int main () {
    auto p1 = SharedPtr<int>(new int(11));
    auto p2 = p1;
    std::cout << p2.count() << std::endl;
    std::cout << *p2 << std::endl;

    return 0;
}