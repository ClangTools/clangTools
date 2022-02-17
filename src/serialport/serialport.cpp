//
// Created by caesar kekxv on 2021/4/11.
//
#include "serialport.h"
#include "logger.h"
#include "easylogging++.h"
#include <mutex>

serialport::serialport(serialport::read_callback_func cb, void *_that) {
    this->readCallbackFunc = cb;
    this->that = _that;
}

bool serialport::_init() {
    stopped.store(false);
    std::condition_variable condition;
    std::unique_lock <std::mutex> _lock{lock};
    stop_finish.store(true);
    pool.enqueue([this](std::condition_variable *condition) -> void {
        stop_finish.store(false);
        condition->notify_all();
        while (!stopped.load()) {
            if (!isOpened()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            for (int i = 5; i > 0; i--) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if (stopped.load()) { break; }
            }
            if (stopped.load()) { break; }
            unsigned char data[2];
            int ret = 0;
            if (stopped.load()) { break; }
            do {
                ret = _read(data, 0, 1, 30);
                if (ret == 1) {
                    push_back(data[0]);
                }
            } while (ret > 0 && !stopped.load());
        }
        stop_finish.store(true);
        stop_cv.notify_all();
    }, &condition);
    // 等待启动线程池
    condition.wait(_lock, [this] { return !stop_finish.load(); });
    return true;
}

serialport::~serialport() = default;

void serialport::push_back(unsigned char data) {
    std::unique_lock <std::mutex> _lock{lock};
    read_data.push_back(data);
    if (read_data.size() > read_data_size) {
        read_data.pop_back();
    }
}

int serialport::available() {
    std::unique_lock <std::mutex> _lock{lock};
    return read_data.size();
}

int serialport::clean() {
    std::unique_lock <std::mutex> _lock{lock};
    auto size = read_data.size();
    read_data.clear();
    return size;
}

int serialport::send(const std::vector<unsigned char> &data, int timeout) {
    return send(data.data(), 0, data.size(), timeout);
}

int serialport::read(std::vector<unsigned char> &data, int data_size, int timeout) {
    unsigned long long time_end = logger::get_time_tick() + (timeout < 0 ? 300 : timeout);
    do {
        if (available() >= data_size) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (time_end > logger::get_time_tick());
    int len = available() >= data_size ? data_size : available();
    if (len == 0)return 0;
    std::unique_lock <std::mutex> _lock_data{lock};
    len = len >= data_size ? data_size : len;
    data.insert(data.end(), read_data.begin(), read_data.begin() + len);
    // std::string message;
    // DLOG(INFO) << "read" << tools::bytes_to_hex_string(data, message);
    read_data.erase(read_data.begin(), read_data.begin() + len);
    return len;
}

int serialport::read(unsigned char *data, int offset, int data_size, int timeout) {
    if (offset < 0) {
        LOG(WARNING) << "offset is " << offset;
        return -1;
    }
    std::vector<unsigned char> rdata;
    int len = read(rdata, data_size, timeout);
    if (len > 0) {
        memcpy(&data[offset], &rdata[0], len);
    }
    return len;
}

serialport *serialport::set_XON_OFF_ANY(int XON, int XOFF, int XANY) {
    return this;
}

void serialport::stop() {
    if (!stopped.load()) {
        stopped.store(true);
    }
    std::unique_lock <std::mutex> lock{lock_wr};
    stop_cv.wait(lock, [this] { return stop_finish.load(); });
}

bool serialport::isOpened() {
    std::unique_lock <std::mutex> _lock{lock_wr};
    return _isOpened();
}
