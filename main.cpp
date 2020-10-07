#include <chrono>
#include <iostream>
#include <vector>

#include "ThreadPool.h"

constexpr int n = 20'000;

void task(int arg) {
    for (volatile int i = 0; i != arg; i++) {
        ;
    }
}

void testThreadPool() {
    std::vector<std::future<void>> ret;
    ThreadPool pool;

    auto begin = std::chrono::high_resolution_clock::now();

    for (volatile int i = 0; i != n; i++) {
        ret.emplace_back(pool.start(task, 10'000));
    }

    for (auto& future : ret) {
        future.get();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "thread pool: " << time.count() / 1000.0 << "s\n";
}

void testThread() {
    std::vector<std::thread> ret;

    auto begin = std::chrono::high_resolution_clock::now();

    for (volatile int i = 0; i != n; i++) {
        ret.emplace_back(task, 10'000);
    }

    for (auto& thread : ret) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    std::cout << "only thread: " << time.count() / 1000.0 << "s\n";
}

int main() {
    using namespace std::literals;

    testThread();
    std::this_thread::sleep_for(5s);
    testThreadPool();
}