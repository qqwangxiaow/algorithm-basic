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
        //读自己生产的地址
        const size_t current_tail = _tail.value.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) % _capacity;
        if (next_tail == _head.value.load(std::memory_order_acquire)) {
            return false;
        }
        _buffer[current_tail] = item;
        _tail.value.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T* item) {
        if (item == nullptr) {
            return false;
        }

        const size_t current_head = _head.value.load(std::memory_order_relaxed);
        if (current_head == _tail.value.load(std::memory_order_acquire)) {
            return false;
        }
        *item = _buffer[current_head];
        const size_t next_head = (current_head + 1) % _capacity;
        _head.value.store(next_head, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return _head.value.load(std::memory_order_acquire) == _tail.value.load(std::memory_order_acquire);
    }

    size_t size() const {
        const size_t head = _head.value.load(std::memory_order_acquire);
        const size_t tail = _tail.value.load(std::memory_order_acquire);
        return (tail + _capacity - head) % _capacity;
    }

private:
    struct alignas(64) AlignedIndex {
        std::atomic<size_t> value{0};
    };
    AlignedIndex _tail;
    AlignedIndex _head;
    std::array<T, Size + 1> _buffer;
    static constexpr size_t _capacity = Size + 1;
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