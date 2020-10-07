#include "ThreadPool.h"

ThreadPool::ThreadPool(unsigned int size) : pool(size), stop(false) {
    for (std::thread& thread : pool) {
        thread = std::thread{std::bind(&ThreadPool::infiniteLoop, this)};
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock{mutex};
        stop = true;
    }

    condition.notify_all();

    for (std::thread& thread : pool) {
        thread.join();
    }
}

void ThreadPool::infiniteLoop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock{mutex};

            condition.wait(lock, [this] { return !taskQueue.empty() || stop; });

            if (stop && taskQueue.empty()) {
                return;
            }

            task = taskQueue.front();
            taskQueue.pop();
        }

        task();
    }
}