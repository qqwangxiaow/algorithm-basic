#include <atomic>
#include <thread> 

class SpinLock {
public:
    SpinLock() = default;
    ~SpinLock() = default;

    inline void lock() {
        while(_flag.test_and_set(std::memory_order_acquire));
    }

    inline void unlock() {
        _flag.clear(std::memory_order_release);
    }


private:
    std::atomic_flag _flag;

}