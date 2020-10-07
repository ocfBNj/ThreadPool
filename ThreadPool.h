#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
    ThreadPool(unsigned int size = std::thread::hardware_concurrency());
    ~ThreadPool();

    // 启动一个任务
    template <typename F, typename... Args>
    auto start(F&& f, Args&&... args)
        -> std::future<decltype(std::forward<F>(f)(std::forward<Args>(args)...))>;

private:
    // 线程池中执行无限循环函数
    std::vector<std::thread> pool;

    // 任务队列
    std::queue<std::function<void()>> taskQueue;

    // 同步
    std::mutex mutex;
    std::condition_variable condition;
    bool stop;

    void infiniteLoop();
};

template <typename F, typename... Args>
inline auto ThreadPool::start(F&& f, Args&&... args)
    -> std::future<decltype(std::forward<F>(f)(std::forward<Args>(args)...))> {
    using ReturnType = decltype(std::forward<F>(f)(std::forward<Args>(args)...));

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    {
        std::unique_lock<std::mutex> lock{mutex};

        if (stop) {
            throw std::runtime_error("start task on stopped ThreadPool");
        }

        taskQueue.emplace([task] { (*task)(); });
    }

    condition.notify_one();

    return task->get_future();
}

#endif // __THREADPOOL_H__