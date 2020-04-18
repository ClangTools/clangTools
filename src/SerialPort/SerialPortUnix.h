//
// Created by caesar kekxv on 2020/4/16.
//

#ifndef TOOLS_SERIALPORTUNIX_H
#define TOOLS_SERIALPORTUNIX_H

#include <termios.h>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

class SerialPortUnix {
public:
    int open(const char *tty_path, int baudrate = 115200, int dataBits = 8, int parity = 'N', int stopBits = 1,
             int flags = 0);

    void close();

    ~SerialPortUnix();

    ssize_t write(std::vector<unsigned char> data);

    ssize_t write(unsigned char *data, ssize_t data_size);

    ssize_t read(std::vector<unsigned char> &data, int timeout = 300);

    ssize_t read(unsigned char *data, ssize_t data_size, int timeout = 300);

    inline bool is_open() const { return fd > 0; }

    inline bool IsOpen() const { return is_open(); }

private:
    int fd = -1;

    static speed_t getBaudrate(int baudrate);
};

#ifdef __cplusplus
}
#endif
#endif //TOOLS_SERIALPORTUNIX_H
