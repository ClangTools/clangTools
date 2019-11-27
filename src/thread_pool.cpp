//
// Created by caesar on 2019/11/21.
//

#include <thread_pool.h>
#include <logger.h>

#ifdef WIN32

#include <process.h>

#endif

using namespace std;


#ifdef __FILENAME__
const char *thread_pool::TAG = __FILENAME__;
#else
const char *thread_pool::TAG = "thread_pool";
#endif

thread_pool::thread_pool(int size) : stoped{false} {
    thread_pool* self = this;
    idlThrNum = size < 1 ? 1 : (size > MAX_THREAD_NUM ? MAX_THREAD_NUM : size);
#ifdef WIN32
    HANDLE handle = (HANDLE) _beginthreadex(nullptr, 0, [](void *arg) -> unsigned {
        auto self = (thread_pool*)arg;
#endif
		thread_pool::init(arg);
#ifdef WIN32
        _endthreadex(0);
        return 0;
    }, (void*)this, 0, nullptr);
    // WaitForSingleObject( handle, INFINITE );
    CloseHandle( handle );
#endif
}

thread_pool::~thread_pool() {
    wait_finish();
}

void thread_pool::wait_finish() {
    // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
    stoped.store(true);
    cv_task.notify_all(); // 唤醒所有线程执行
    for (std::thread &thread : pool) {
        //thread.detach(); // 让线程“自生自灭”
        if (thread.joinable())
            thread.join(); // 等待任务结束， 前提：线程一定会执行完
    }
    while (!tasks.empty()) {
        std::unique_lock<std::mutex> lock{this->m_lock};
        std::move(this->tasks.front())(); // 取一个 task
        this->tasks.pop();
    }
}

unsigned thread_pool::init(void *arg) {
    auto self = (thread_pool*)arg;
    for (int size = 0; size < self->idlThrNum; ++size) {   //初始化线程数量
        self->pool.emplace_back(
                [self] { // 工作线程函数
                    while (!self->stoped) {
                        std::function<void()> task;
                        {   // 获取一个待执行的 task
                            // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
                            std::unique_lock<std::mutex> lock{self->m_lock};
                            self->cv_task.wait(lock,
                                               [self] {
                                                   return self->stoped.load() || !self->tasks.empty();
                                               }
                            ); // wait 直到有 task
                            if (self->stoped && self->tasks.empty()) {
                                return;
                            }
                            task = std::move(self->tasks.front()); // 取一个 task
                            self->tasks.pop();
                        }
                        self->idlThrNum--;
                        task();
                        self->idlThrNum++;
                    }
                }
        );
    }
    return 0;
}
