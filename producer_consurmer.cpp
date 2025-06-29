#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

const int MAX_BUFFER_SIZE = 5; // 缓冲区最大容量
std::queue<int> buffer;        // 缓冲区

std::mutex mtx;
std::condition_variable cv_producer;
std::condition_variable cv_consumer;

void producer() {
    int data = 0;
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_producer.wait(lock, []{ return buffer.size() < MAX_BUFFER_SIZE; }); // 缓冲区未满才能生产

        // 生产数据
        buffer.push(data);
        std::cout << "Produced: " << data << std::endl;
        data++;

        cv_consumer.notify_one(); // 通知消费者
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟生产耗时
    }
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_consumer.wait(lock, []{ return !buffer.empty(); }); // 缓冲区非空才能消费

        // 消费数据
        int data = buffer.front();
        buffer.pop();
        std::cout << "Consumed: " << data << std::endl;

        cv_producer.notify_one(); // 通知生产者
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(150)); // 模拟消费耗时
    }
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);

    t1.join();
    t2.join();

    return 0;
}