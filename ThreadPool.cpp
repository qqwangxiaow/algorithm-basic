#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <stdexcept>
#include <iostream>

class ThreadPool {
private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _tasks;
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _stop;

public:
    ThreadPool(size_t n) : _stop(false) {
        for (size_t i = 0; i < n; ++i) {
            _workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        _cv.wait(lock, [this] { return _stop || !_tasks.empty(); });
                        if (_stop && _tasks.empty()) {
                            break;
                        }
                        task = std::move(_tasks.front());
                        _tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _stop = true;
        }
        _cv.notify_all();
        for (std::thread& worker : _workers) {
            worker.join();
        }
    }

    template<typename F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_stop) {
                // 不抛异常
                // return std::future<return_type>();
                // 使用者 future.valid()判断
                throw std::runtime_error("enqueue on stopped thread pool");
            }
            _tasks.emplace([task]() { (*task)(); });
        }
        _cv.notify_one();
        return res;
    }
};

//test
//g++ -std=c++17 Threadpool.cpp -o Threadpool -pthread
int main() {
    ThreadPool pool(4); // 创建4个线程的线程池

    auto future1 = pool.enqueue([]() {
        std::cout << "Hello from thread pool!" << std::endl;
        return 123;
    });

    // 提交带参数的任务
    auto future2 = pool.enqueue([](int a, int b) {
        return a + b;
    }, 10, 20);

    // 提交多个任务
    std::vector<std::future<int>> results;
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.enqueue([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        }));
    }

    // 获取结果
    std::cout << "future1 result: " << future1.get() << std::endl;
    std::cout << "future2 result: " << future2.get() << std::endl;
    for (int i = 0; i < results.size(); ++i) {
        std::cout << "Task " << i << " result: " << results[i].get() << std::endl;
    }

    return 0;
}
