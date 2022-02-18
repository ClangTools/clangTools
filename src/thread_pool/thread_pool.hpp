#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifdef _MSC_VER
#include "ThreadPool.hpp"
#define thread_pool ThreadPool;
#else

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "threadpool.h"
#include <iostream>

class thread_pool {
public:
    explicit thread_pool(size_t);

    template<class F, class... Args>
    auto enqueue(F &&f, Args &&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;

    ~thread_pool();

    void shutdown();

private:
    threadpool_t *pool = nullptr;
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;


    typedef struct thread_arg {
        std::mutex mutex;
        bool is_start = false;
        bool is_finish = false;
        std::condition_variable cv;
        std::shared_ptr<std::packaged_task<void()>> task;
        void *resp;
    } ThreadArg;
};

// the constructor just launches some amount of workers
inline thread_pool::thread_pool(size_t threads)
        : stop(false) {
    pool = threadpool_create(threads);
}

// add new work item to the pool
template<class F, class... Args>
auto thread_pool::enqueue(F &&f, Args &&... args)
-> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto *t = new ThreadArg();
    t->task = (std::shared_ptr<std::packaged_task<void()>>) task;
    t->is_start = false;
    auto resp = task->get_future();
    auto ret = threadpool_add(pool, [](void *arg) {
        auto *t = (ThreadArg *) arg;
        auto task = (std::shared_ptr<std::packaged_task<return_type()>>) t->task;
        (*task)();
        delete t;
    }, (void *) t, 0);
    if (ret != 0) {
        (*task)();
    }
    return resp;
}

// the destructor joins all threads
inline thread_pool::~thread_pool() {
    shutdown();
}

inline void thread_pool::shutdown() {
    // delete threadPool;
    if (pool != nullptr) {
        threadpool_destroy(pool, threadpool_graceful);
    }
    pool = nullptr;
}

#endif
#endif