# C++线程池

[toc]

C++线程池的简单实现。

完整代码：<https://github.com/ocfBNj/ThreadPool>

## 介绍

**线程池**（英语：thread pool）：一种[线程](https://zh.wikipedia.org/wiki/线程)使用模式。线程过多会带来调度开销，进而影响缓存局部性和整体性能。而线程池维护着多个线程，等待着监督管理者分配可并发执行的任务。这避免了在处理短时间任务时创建与销毁线程的代价。线程池不仅能够保证内核的充分利用，还能防止过分调度。线程数一般取cpu数量+2比较合适，线程数过多会导致额外的线程切换开销。

上面是维基百科对线程池的介绍。

## 使用C++实现线程池

### 1. 查看系统支持的最大线程数量

~~~cpp
unsigned int size = std::thread::hardware_concurrency();
~~~

使用[std::thread::hardware_concurrency()](https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency)获得系统支持的最大线程数量。

### 2. 线程池的方法和数据结构

~~~cpp
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
~~~

### 3. 线程池的构造函数

~~~cpp
ThreadPool::ThreadPool(unsigned int size) : pool(size), stop(false) {
    for (std::thread& thread : pool) {
        thread = std::thread{std::bind(&ThreadPool::infiniteLoop, this)};
    }
}
~~~

因为线程的创建和摧毁需要时间开销。对于一个高效的线程池，一次性创建`size`个线程，并且在之后不再创建新的线程或摧毁旧的线程。**每个线程池中的线程中通过无限循环的函数从任务队列中取走并执行任务。**

### 4. 无限循环的函数

~~~cpp
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
~~~

当任务队列为空时，阻塞直到任务队列不为空或线程池被停止（即将被销毁）。

当线程池被停止时，执行完未完成的任务，然后退出该无限循环函数。

从任务队列取出一个任务后，执行它。

### 5. 线程池的析构函数

~~~cpp
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
~~~

停止线程池并通知线程池中的线程。

等待线程池中的线程执行完毕。

### 6. start函数

~~~cpp
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
~~~

start函数将一个**任务**（函数）添加到任务队列中，并通过[std::future](https://en.cppreference.com/w/cpp/thread/future)获得该任务的返回值。

[std::packaged_task](https://en.cppreference.com/w/cpp/thread/packaged_task)打包一个[可调用](https://en.cppreference.com/w/cpp/named_req/Callable)目标（如函数，lambda表达式，bind表达式或其他函数对象），它的返回值可以通过[std::future](https://en.cppreference.com/w/cpp/thread/future)获得。

使用智能指针[std::shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr)共享[std::packaged_task](https://en.cppreference.com/w/cpp/thread/packaged_task)对象（因为它不可拷贝，且在lambda表达式中被共享）。

最后通过[std::condition_variable](https://en.cppreference.com/w/cpp/thread/condition_variable)通知线程池中的线程有新任务到达。

## 测试

~~~cpp
#include <iostream>
#include <vector>

#include "ThreadPool.h"

int main() {
    ThreadPool pool;
    std::vector<std::future<int>> ret;

    for (int i = 0; i != 1000; i++) {
        ret.push_back(pool.start([](int index) { return index; }, i));
    }

    for (auto& future : ret) {
        std::cout << future.get() << "\n";
    }
}
~~~

简单的测试代码。

## 参考资料

<https://stackoverflow.com/questions/15752659/thread-pooling-in-c11>

<https://github.com/progschj/ThreadPool>

<https://cppreference.com>
