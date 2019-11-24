//
// Created by caesar on 2019/11/21.
//

#include <thread_pool.h>
#include <logger.h>

using namespace std;


#ifdef __FILENAME__
const char *thread_pool::TAG = __FILENAME__;
#else
const char *thread_pool::TAG = "thread_pool";
#endif

thread_pool::thread_pool(int size) : stoped{false} {
    idlThrNum = size < 1 ? 1 : (size > MAX_THREAD_NUM ? MAX_THREAD_NUM : size);
    for (size = 0; size < idlThrNum; ++size) {   //初始化线程数量
        pool.emplace_back(
                [this] { // 工作线程函数
                    while (!this->stoped) {
                        std::function<void()> task;
                        {   // 获取一个待执行的 task
                            // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
                            std::unique_lock<std::mutex> lock{this->m_lock};
                            this->cv_task.wait(lock,
                                               [this] {
                                                   return this->stoped.load() || !this->tasks.empty();
                                               }
                            ); // wait 直到有 task
                            if (this->stoped && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front()); // 取一个 task
                            this->tasks.pop();
                        }
                        idlThrNum--;
                        task();
                        idlThrNum++;
                    }
                }
        );
    }
}

thread_pool::~thread_pool() {
    stoped.store(true);
    cv_task.notify_all(); // 唤醒所有线程执行
    // usleep(10);
    for (std::thread &thread : pool) {
        //thread.detach(); // 让线程“自生自灭”
        if (thread.joinable())
            thread.join(); // 等待任务结束， 前提：线程一定会执行完
    }
}
