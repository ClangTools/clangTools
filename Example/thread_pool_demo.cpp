//
// Created by caesar on 2019/11/21.
//

#include <thread_pool.h>
#include <logger.h>

using namespace std;

#define _TAG __FILENAME__,__LINE__

int main() {
    auto start_tick = logger::get_time_tick();
    logger::instance()->console_show = true;
    logger::instance()->min_level = logger::log_rank_DEBUG;
    try {
        logger::instance()->i(_TAG, "time:  %lld", logger::get_time_tick() - start_tick);
        thread_pool executor{50};
        std::future<std::string> fh = executor.commit([](long long start_tick) -> std::string {
            logger::instance()->d(_TAG, "hello, fh !  %d", std::this_thread::get_id());
            return "hello,fh ret !";
        }, start_tick);
        std::future<std::string> fh1 = executor.commit([](long long start_tick) -> std::string {
            logger::instance()->d(_TAG, "hello, fh !  %d", std::this_thread::get_id());
            return "hello,fh ret !";
        }, start_tick);
        std::future<std::string> fh2 = executor.commit([](long long start_tick) -> std::string {
            logger::instance()->d(_TAG, "hello, fh !  %d", std::this_thread::get_id());
            return "hello,fh ret !";
        }, start_tick);
        logger::instance()->d(_TAG, "%s", fh.get().c_str());
        logger::instance()->d(_TAG, "%s", fh1.get().c_str());
        logger::instance()->d(_TAG, "%s", fh2.get().c_str());


        logger::instance()->i(_TAG, "time:  %lld", logger::get_time_tick() - start_tick);
        return 0;
    }
    catch (std::exception &e) {
        std::cout << "some unhappy happened...  " << std::this_thread::get_id() << e.what() << std::endl;
    }

}