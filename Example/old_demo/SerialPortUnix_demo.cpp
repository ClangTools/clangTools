//
// Created by caesar kekxv on 2020/4/16.
//

#include <SerialPort.h>
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
        serialPortUnix.write({0xA1, 0x4D, 0x21, 0x11, 0x00, 0xED});
        std::vector<unsigned char> data;
        auto start = logger::get_time_tick();
        int ret = serialPortUnix.read(data,500);
        auto end = logger::get_time_tick();
        logger::instance()->d(__FILENAME__, __LINE__, "read", data.data(), data.size());
        logger::instance()->i(__FILENAME__, __LINE__, "read size : %d ; %lld", ret, end - start);
    }
    serialPortUnix.close();
    return 0;
}

