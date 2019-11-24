//
// Created by caesar on 2019/11/21.
//
// 使用的是 https://www.jianshu.com/p/eec63026f8d0 内代码
// 网站参考代码 https://github.com/progschj/ThreadPool
//

#ifndef TOOLS_THREAD_POOL_H
#define TOOLS_THREAD_POOL_H

#include <thread>
#include <map>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


#ifdef WIN32
#ifdef _Tools_HEADER_
#define DLL_thread_pool_Export  __declspec(dllexport)
#else
#define DLL_thread_pool_Export  __declspec(dllimport)
#endif
#else
#define DLL_thread_pool_Export
#endif

class DLL_thread_pool_Export thread_pool {

    static const char *TAG;

    static const int MAX_THREAD_NUM = 512;
    using Task = std::function<void()>;
    // 线程池
    std::vector<std::thread> pool;
    // 任务队列
    std::queue<Task> tasks;
    // 同步
    std::mutex m_lock;
    // 条件阻塞
    std::condition_variable cv_task;
    // 是否关闭提交
    std::atomic<bool> stoped{false};
    //空闲线程数量
    std::atomic<int> idlThrNum{1};

public:

    explicit thread_pool(int size = 4);

    ~thread_pool();

public:
    template<class F, class... Args>
    auto commit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        if (stoped.load())    // stop == true ??
            throw std::runtime_error("commit on ThreadPool is stopped.");

        using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
        auto task = std::make_shared<std::packaged_task<RetType()> >(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );    // wtf !
        std::future<RetType> future = task->get_future();
        {    // 添加任务到队列
            //对当前块的语句加锁  lock_guard 是 mutex 的 stack 封装类，构造的时候 lock()，析构的时候 unlock()
            std::lock_guard<std::mutex> lock{m_lock};
            tasks.emplace(
                    [task]() { // push(Task{...})
                        (*task)();
                    }
            );
        }
        cv_task.notify_one(); // 唤醒一个线程执行

        return future;
    }

    //空闲线程数量
    int idlCount() { return idlThrNum; }

};


#endif //TOOLS_THREAD_POOL_H
