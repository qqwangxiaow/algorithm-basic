#include <atomic>
#include <utility>

template <typename T>

class SharedPtr {
public:
    explicit SharedPtr(T* p = nullptr) :
            _ptr(p),
            _ref_count(p ? new std::atomic<int>(1) : nullptr) {
    }

    SharedPtr(T* p, std::atomic<int>* ref_count, void* control_block) :
            _ptr(p),
            _ref_count(ref_count),
            _control_block(control_block) {
    }

    ~SharedPtr() {
        release();
    }

    SharedPtr(const SharedPtr& other) : _ptr(other._ptr), _ref_count(other._ref_count) {
        if (_ref_count) {
            ++(*_ref_count);
        }
    }

    SharedPtr(SharedPtr&& other) noexcept : _ptr(other._ptr), _ref_count(other._ref_count) {
        other._ptr = nullptr;
        other._ref_count = nullptr;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release();
            _ptr = other._ptr;
            _ref_count = other._ref_count;
            if (_ref_count) {
                ++(*_ref_count);
            }
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            _ptr = other._ptr;
            _ref_count = other._ref_count;
            other._ptr = nullptr;
            other._ref_count = nullptr;
        }
        return *this;
    }

    T& operator*() const {
        return *_ptr;
    }

    T* operator->() const {
        return _ptr;
    }

    T* get() const {
        return _ptr;
    }

    int use_count() const {
        return _ref_count ? _ref_count->load() : 0;
    }

    bool unique() const {
        return use_count() == 1;
    }

    void reset(T* p = nullptr) {
        if (_ptr != p) {
            release();
            if (p) {
                _ptr = p;
                _ref_count = new std::atomic<int>(1);
            }
        }
    }

    void swap(SharedPtr& other) noexcept {
        std::swap(_ptr, other._ptr);
        std::swap(_ref_count, other._ref_count);
    }

    friend void swap(SharedPtr& a, SharedPtr& b) noexcept {
        a.swap(b);
    }

    operator bool() const {
        return _ptr != nullptr;
    }

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

private:
    void release() {
        if (_ref_count) {
            if (--(*_ref_count) == 0) {
                if (_control_block) {
                    // 用make_shared方式分配的，整体delete
                    delete static_cast<ControlBlock*>(_control_block);
                } else {
                    // 普通new出来的，只delete对象和计数
                    delete _ptr;
                    delete _ref_count;
                }
            }
            _ptr = nullptr;
            _ref_count = nullptr;
            _control_block = nullptr;
        }
    }

    struct ControlBlock {
        std::atomic<int> ref_count;
        T object;

        template <typename... Args>
        ControlBlock(Args&&... args) : ref_count(1), object(std::forward<Args>(args)...) {
        }
    };

    T* _ptr = nullptr;
    std::atomic<int>* _ref_count = nullptr;
    void* _control_block = nullptr;
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    using ControlBlock = typename SharedPtr<T>::ControlBlock;
    auto* block = new ControlBlock(std::forward<Args>(args)...);
    return SharedPtr<T>(&block->object, &block->ref_count, block);
}

int main() {
    return 0;
}