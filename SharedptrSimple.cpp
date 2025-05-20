#include <atomic>
#include <string>


template<typename T>
class SharedPtr {
public:
    explicit SharedPtr(T* data = nullptr) : _data(data), _ref_count(_data ? new std::atomic<int>(1) : nullptr) {}

    ~SharedPtr() {
        release();
    }

    SharedPtr(const SharedPtr& other) : _data(other._data), _ref_count(other._ref_count) {
        if(_ref_count) {
            ++(*_ref_count);
        };
    }

    SharedPtr(SharedPtr&& other) noexcept : _data(other._data), _ref_count(other._ref_count)  {
        other._data = nullptr;
        other._ref_count = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            this->_data = other._data;
            this->_ref_count = other._ref_count;
            other._data = nullptr;
            other._ref_count = nullptr;
        }
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release();
            this->_data = other._data;
            this->_ref_count = other._ref_count;
            if (_ref_count) {
                ++(*_ref_count);
            }
        }
        return *this;
    };

    T* get() const {
        return _data;
    }
 
    T* operator->() const {
        return _data;
    }

    T& operator*() const {
        return *_data;
    }

    operator bool() const {
        return _data != nullptr;
    }

    bool unique() const {
        return use_count() == 1;
    }

    int use_count() const {
        return _ref_count ? _ref_count->load() : 0;
    }


    void reset(T* data = nullptr) {
        if (_data != data) {
            release();
            if (data) {
                _data = data;
                _ref_count = new std::atomic<int>(1);
            } else {
                _data = nullptr;
                _ref_count = nullptr;
            }
        }
    }
    

private:
    void release() {
        if (_ref_count) {
            if (_ref_count->fetch_sub(1) == 1) {
                delete _data;
                delete _ref_count;
                _data = nullptr;
                _ref_count = nullptr;
            }
        }
    }
    T* _data;
    std::atomic<int>* _ref_count;

};

int main() {

    auto aa = SharedPtr<int>(new int(11));

}