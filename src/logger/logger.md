# logger tool

## 注意
使用 `-D_LOGGER_USE_THREAD_POOL_` 开启线程池

如果使用线程池，则:
1. 使用 `instance()` 情况下必须调用 `free_instance()`
1. 自己创建对象情况下，请调用 `wait_finish()`



## Example

```c++
#include <string>
#include <logger.h>

using namespace std;

int main(int argc, char **argv) {
    auto log = logger::instance();
    log->min_level = logger::log_rank_t::log_rank_DEBUG;
    std::string logpath = logger::get_local_path() + logger::path_split + "log";
    logger::mk_dir(logpath);
    std::string logfile = logpath + logger::path_split + "main.log";
    log->open((logfile).c_str());
    log->logger_files_max_size = 5;
    log->logger_file_max_size = 100;

    for(int i=0;i<10000;i++) {
        log->d(("Example/main.cpp" + to_string(__LINE__)).c_str(), "%03d:%s", i,"DEBUG");
        log->i(("Example/main.cpp" + to_string(__LINE__)).c_str(), "%03d:%s", i,"INFO");
        log->w(("Example/main.cpp" + to_string(__LINE__)).c_str(), "%03d:%s", i,"WARNING");
        log->e(("Example/main.cpp" + to_string(__LINE__)).c_str(), "%03d:%s", i,"ERROR");
        log->f(("Example/main.cpp" + to_string(__LINE__)).c_str(), "%03d:%s", i,"FATAL");
    }
    return 0;
}
```
## Interface

### control console show

bool console_show = true;

### control log file max data

size_t logger_file_max_size = 1024 * 1024;

### control log files max count

size_t logger_files_max_size = 50;

