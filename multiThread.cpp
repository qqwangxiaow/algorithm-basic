#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>


std::mutex m;
std::condition_variable cv;
int i = 0;

void print1() {
    while (1) {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [](){return i % 3 == 0;});
        std::cout << "1" << std::endl;
        i++;
        cv.notify_all();
    }
}
void print2() {
    while(1) {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [](){return i % 3 == 1;});
        std::cout << "2" << std::endl;
        i++;
        cv.notify_all();
    }
}
void print3() {
    while(1) {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [](){return i % 3 == 2;});
        std::cout << "3" << std::endl;
        i++;
        cv.notify_all();
    }

}

int main () {
    std::thread t1(print1);
    std::thread t2(print2);
    std::thread t3(print3);
    t1.join();
    t2.join();
    t3.join();
}