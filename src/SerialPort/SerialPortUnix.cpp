//
// Created by caesar kekxv on 2020/4/16.
//
/*
 * Copyright 2009-2011 Cedric Priscal
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SerialPortUnix.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <logger.h>
#include <poll.h>
#include <poll_tool.h>

speed_t SerialPortUnix::getBaudrate(int baudrate) {
    switch (baudrate) {
        case 0:
            return B0;
        case 50:
            return B50;
        case 75:
            return B75;
        case 110:
            return B110;
        case 134:
            return B134;
        case 150:
            return B150;
        case 200:
            return B200;
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 1800:
            return B1800;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
#ifndef __apple_build_version__
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
#endif
        default:
            return -1;
    }
}

int SerialPortUnix::open(const char *path, int baudrate, int dataBits, int parity, int stopBits, int flags) {
    if (path == nullptr)return -1;
    if (std::string(path).find("ttyS0") != std::string::npos)return -1;
    speed_t speed;
    {
        speed = getBaudrate(baudrate);
        if (speed == -1) {
            /* TODO: throw an exception */
            logger::instance()->e(__FILENAME__, __LINE__, "Invalid baudrate");
            return -1;
        }
    }

    /* Opening device */
    {
        logger::instance()->d(__FILENAME__, __LINE__, "Opening serial port %s with flags 0x%x", path,
                              O_RDWR | O_NOCTTY | O_NONBLOCK | flags);
        fd = ::open(path, O_RDWR | flags);
        if (fd == -1) {
            /* Throw an exception */
            logger::instance()->e(__FILENAME__, __LINE__, "Cannot open port");
            /* TODO: throw an exception */
            return -1;
        }
    }

    /* Configure device */
    {
        struct termios cfg;
        if (tcgetattr(fd, &cfg)) {
            logger::instance()->e(__FILENAME__, __LINE__, "tcgetattr() failed");
            close();
            /* TODO: throw an exception */
            return -1;
        }

        cfmakeraw(&cfg);
        cfsetispeed(&cfg, speed);
        cfsetospeed(&cfg, speed);


        cfg.c_cflag &= ~CSIZE;
        switch (dataBits) {
            case 5:
                cfg.c_cflag |= CS5;    //使用5位数据位
                break;
            case 6:
                cfg.c_cflag |= CS6;    //使用6位数据位
                break;
            case 7:
                cfg.c_cflag |= CS7;    //使用7位数据位
                break;
            case 8:
                cfg.c_cflag |= CS8;    //使用8位数据位
                break;
            default:
                cfg.c_cflag |= CS8;
                break;
        }

        switch (parity) {
            case 'O':
                cfg.c_cflag |= (PARODD | PARENB);   //奇校验
                break;
            case 'E':
                cfg.c_iflag &= ~(IGNPAR | PARMRK); // 偶校验
                cfg.c_iflag |= INPCK;
                cfg.c_cflag |= PARENB;
                cfg.c_cflag &= ~PARODD;
                break;
            case 'N':
            default:
                cfg.c_cflag &= ~PARENB;
                break;
        }

        switch (stopBits) {
            case 1:
                cfg.c_cflag &= ~CSTOPB;    //1位停止位
                break;
            case 2:
                cfg.c_cflag |= CSTOPB;    //2位停止位
                break;
            default:
                cfg.c_cflag &= ~CSTOPB;    //1位停止位
                break;
        }

        if (tcsetattr(fd, TCSANOW, &cfg)) {
            logger::instance()->e(__FILENAME__, __LINE__, "tcsetattr() failed");
            close();
            return -1;
        }
    }
    return fd;
}

/*
 * Class:     cedric_serial_SerialPort
 * Method:    close
 * Signature: ()V
 */
void SerialPortUnix::close() {
    if (fd > 0)::close(fd);
    fd = -1;
}

SerialPortUnix::~SerialPortUnix() {
    close();
}

ssize_t SerialPortUnix::write(std::vector<unsigned char> data) {
    return write(data.data(), data.size());
}

ssize_t SerialPortUnix::write(unsigned char *data, ssize_t data_size) {
    if (!is_open())return -1;
    return ::write(fd, data, data_size);
}


ssize_t SerialPortUnix::read(std::vector<unsigned char> &data, int timeout) {
    unsigned char _data[1024]{};
    ssize_t ret = read(_data, sizeof(_data), timeout);
    if (ret > 0) {
        data.insert(data.end(), _data, _data + ret);
    }
    return ret;
}

ssize_t SerialPortUnix::read(unsigned char *data, ssize_t data_size, int timeout) {
    if (!is_open())return -1;
    poll_tool pollTool(&fd);

    ssize_t wCount = 0;
    unsigned char str[258]{};
    auto end = logger::get_time_tick() + timeout;
    if (pollTool.check_read_count((int) (end - logger::get_time_tick())) <= 0) {
        return 0;
    }
    do {
        int ret = ::read(fd, str, sizeof(str) / sizeof(char));
        if (ret >= 0) {
            if (data_size < wCount + ret) {
                logger::instance()->e(__FILENAME__, __LINE__, "data size error");
                return -1;
            }
            memcpy(&data[wCount], str, ret);
            wCount += ret;
        }
    } while (end > logger::get_time_tick() && pollTool.check_read_count(5) > 0);
    return wCount;
}
