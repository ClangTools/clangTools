//
// Created by caesar kekxv on 2020/4/16.
//

#include <SerialPortUnix.h>
#include <logger.h>

int main(int argc, char *argv[]) {
    logger::instance()->init_default();
    if (argc != 2) {
        logger::instance()->e(__FILENAME__, __LINE__, "错误参数");
        return 0;
    }
    SerialPortUnix serialPortUnix;
    serialPortUnix.open(argv[1], 115200, 8, 'n', 1, 0);
    if (serialPortUnix.is_open()) {
        serialPortUnix.write({0x1a, 0x25, 0xc1, 0x11});
        std::vector<unsigned char> data;
        auto start = logger::get_time_tick();
        int ret = serialPortUnix.read(data);
        auto end = logger::get_time_tick();
        data.erase(data.end() - 1);
        data.push_back(0);
        logger::instance()->i(__FILENAME__, __LINE__, "read size : %d ; %s ; %lld", ret, data.data(), end - start);
    }
    serialPortUnix.close();
    return 0;
}

