#include <atomic>
#include <iostream>
#include <cstddef>
#include <array>

template<typename T, size_t Size> 
class SPSCQueue {
public:
    static_assert(Size > 0, "Size must be greater than 0");

    SPSCQueue() = default;
    ~SPSCQueue() = default;
    SPSCQueue(const SPSCQueue& other) = delete;
    SPSCQueue& operator=(const SPSCQueue& other) = delete;

    bool push(const T& item) {
        const size_t current_tail = _tail.load(std::memory_order_relaxed);
        const size_t current_head = _head.load(std::memory_order_acquire);
        if (current_tail - current_head == _capacity) {
            return false;
        }
        _buffer[current_tail % _capacity] = item;
        _tail.store(current_tail + 1, std::memory_order_release);
        return true;
    }

    bool pop(T* item) {
        if (item == nullptr) {
            return false;
        }

        const size_t current_head = _head.load(std::memory_order_relaxed);
        const size_t current_tail = _tail.load(std::memory_order_acquire);
        if (current_head == current_tail) {
            return false;
        }
        *item = _buffer[current_head % _capacity];
        _head.store(current_head + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return _head.load(std::memory_order_acquire) == _tail.load(std::memory_order_acquire);
    }

    bool full() const {
        const size_t head = _head.load(std::memory_order_acquire);
        const size_t tail = _tail.load(std::memory_order_acquire);
        return tail - head == _capacity;
    }

    size_t size() const {
        const size_t head = _head.load(std::memory_order_acquire);
        const size_t tail = _tail.load(std::memory_order_acquire);
        return tail - head;
    }

private:
    alignas(64) std::atomic<size_t> _tail{0};
    alignas(64) std::atomic<size_t> _head{0};
    std::array<T, Size> _buffer;
    static constexpr size_t _capacity = Size;
};

int main () {
    SPSCQueue<int, 10> queue;
    queue.push(1);
    queue.push(2);
    int a;
    queue.pop(&a);
    std::cout << "a=:" << a << std::endl;
    return 0;

}