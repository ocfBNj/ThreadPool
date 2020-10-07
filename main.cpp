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