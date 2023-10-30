//
// Created by caesar kekxv on 2021/4/12.
//

#include "serialportUnix.h"
#include "easylogging++.h"
#include "serialport.h"


#ifdef ENABLE_POLL

#include "poll.h"
#include "logger.h"

#endif


serialport *serialport::newSerialport() {
    return new serialportUnix();
}

void serialport::deleteSerialport(serialport *&_serialport) {
    if (_serialport != nullptr) {
        (_serialport)->stop();
        (_serialport)->close();
        delete (serialportUnix *) _serialport;
    }
    _serialport = nullptr;
}

serialportUnix::serialportUnix() {
    _init();
}

serialportUnix::~serialportUnix() = default;

bool serialportUnix::open(const char *path) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    LOG(INFO) << "open serial port : " << path;
    hCom = ::open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    LOG(INFO) << "serial port isOpened() : " << (_isOpened() ? "true" : "false");
    if (_isOpened()) {
        old_tio_flag = true;
        if (tcgetattr(hCom, &old_tio) != 0) {
            LOG(ERROR) << "get serial port old_tio : " << strerror(errno);
            old_tio_flag = false;
        }
    }
    return _isOpened();
}

bool serialportUnix::close() {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (_isOpened()) {
        if (old_tio_flag) {
            tcsetattr(hCom, TCSAFLUSH, &old_tio);
        }
        ::close(hCom);
    }
    hCom = -1;
    return _isOpened();
}

bool serialportUnix::_isOpened() {
    return hCom > 0;
}

serialport *serialportUnix::set_baud_rate(serialport::BaudRate baud_rate) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (!_isOpened()) {
        return this;
    }
    struct termios new_tio{};
    cfmakeraw(&new_tio);
    tcgetattr(hCom, &new_tio);
    unsigned long _baud_rate = B0;
    switch (baud_rate) {
        case BaudRate::BR50:
            _baud_rate = B50;
            break;
        case BaudRate::BR75:
            _baud_rate = B75;
            break;
        case BaudRate::BR134:
            _baud_rate = B134;
            break;
        case BaudRate::BR150:
            _baud_rate = B150;
            break;
        case BaudRate::BR200:
            _baud_rate = B200;
            break;
        case BaudRate::BR300:
            _baud_rate = B300;
            break;
        case BaudRate::BR600:
            _baud_rate = B600;
            break;
        case BaudRate::BR1200:
            _baud_rate = B1200;
            break;
        case BaudRate::BR1800:
            _baud_rate = B1800;
            break;
        case BaudRate::BR2400:
            _baud_rate = B2400;
            break;
        case BaudRate::BR4800:
            _baud_rate = B4800;
            break;
        case BaudRate::BR9600:
            _baud_rate = B9600;
            break;
        case BaudRate::BR19200:
            _baud_rate = B19200;
            break;
        case BaudRate::BR38400:
            _baud_rate = B38400;
            break;
        case BaudRate::BR57600:
            _baud_rate = B57600;
            break;
        case BaudRate::BR115200:
            _baud_rate = B115200;
            break;
        case BaudRate::BR230400:
            _baud_rate = B230400;
            break;
#ifndef __apple_build_version__
            case BaudRate::BR460800:
                _baud_rate = B460800;
                break;
            case BaudRate::BR500000:
                _baud_rate = B500000;
                break;
            case BaudRate::BR576000:
                _baud_rate = B576000;
                break;
            case BaudRate::BR921600:
                _baud_rate = B921600;
                break;
            case BaudRate::BR1000000:
                _baud_rate = B1000000;
                break;
            case BaudRate::BR1152000:
                _baud_rate = B1152000;
                break;
            case BaudRate::BR1500000:
                _baud_rate = B1500000;
                break;
            case BaudRate::BR2000000:
                _baud_rate = B2000000;
                break;
            case BaudRate::BR2500000:
                _baud_rate = B2500000;
                break;
            case BaudRate::BR3000000:
                _baud_rate = B3000000;
                break;
            case BaudRate::BR3500000:
                _baud_rate = B3500000;
                break;
            case BaudRate::BR4000000:
                _baud_rate = B4000000;
                break;
#endif
        default:
            break;
    }
    cfsetspeed(&new_tio, _baud_rate);
    if ((tcsetattr(hCom, TCSAFLUSH, &new_tio)) != 0) {
        LOG(ERROR) << "set_baud_rate " << _baud_rate << " : " << strerror(errno);
    }
    return this;
}

serialport *serialportUnix::set_data_bits(serialport::DataBits data_bits) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (!_isOpened()) {
        return this;
    }
    struct termios new_tio{};
    cfmakeraw(&new_tio);
    tcgetattr(hCom, &new_tio);


    int _data_bits[] = {0x00, 0x00, 0x00, 0x00, CS5, CS6, CS7, CS8};
    new_tio.c_cflag &= ~0x30;
    new_tio.c_cflag |= _data_bits[data_bits];

    if ((tcsetattr(hCom, TCSAFLUSH, &new_tio)) != 0) {
        LOG(INFO) << "set_data_bits" << data_bits << " " << strerror(errno);
    }
    return this;
}

serialport *serialportUnix::set_stop_bits(serialport::StopBits stop_bits) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (!_isOpened()) {
        return this;
    }
    struct termios new_tio{};
    cfmakeraw(&new_tio);
    tcgetattr(hCom, &new_tio);

    // stop bits
    if (stop_bits == StopBits2) {
        new_tio.c_cflag |= CSTOPB;
    } else {
        new_tio.c_cflag &= ~CSTOPB;
    }

    if ((tcsetattr(hCom, TCSAFLUSH, &new_tio)) != 0) {
        LOG(INFO) << "set_data_bits" << stop_bits << " " << strerror(errno);
    }
    return this;
}

serialport *serialportUnix::set_parity(serialport::Parity parity) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (!_isOpened()) {
        return this;
    }
    struct termios new_tio{};
    cfmakeraw(&new_tio);
    tcgetattr(hCom, &new_tio);

    // parity
    if (parity == ParityNone) {
        new_tio.c_cflag &= ~PARENB;
    } else {
        new_tio.c_cflag |= PARENB;

        if (parity == ParityMark) {
            new_tio.c_cflag |= PARMRK;
        } else {
            new_tio.c_cflag &= ~PARMRK;
        }
        if (parity == ParityOdd) {
            new_tio.c_cflag |= PARODD;
        } else {
            new_tio.c_cflag &= ~PARODD;
        }
    }

    if ((tcsetattr(hCom, TCSAFLUSH, &new_tio)) != 0) {
        LOG(INFO) << "set_parity" << parity << " " << strerror(errno);
    }
    return this;
}

serialport *serialportUnix::set_XON_OFF_ANY(int xon, int xoff, int xany) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (!_isOpened()) {
        return this;
    }
    struct termios new_tio{};
    cfmakeraw(&new_tio);
    tcgetattr(hCom, &new_tio);

    new_tio.c_iflag |= (xon ? IXON : 0)
                       | (xoff ? IXOFF : 0)
                       | (xany ? IXANY : 0);

    if ((tcsetattr(hCom, TCSAFLUSH, &new_tio)) != 0) {
        LOG(INFO) << "set_XON_OFF_ANY" << xon << " " << xoff << " " << xany << " " << strerror(errno);
    }
    return this;
}

int serialportUnix::send(const unsigned char *data, int offset, int data_size, int timeout) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (!_isOpened()) {
        return -1;
    }
    size_t l = ::write(hCom, &data[offset], data_size);
    std::string message;
    logger::bytes_to_hex_string((unsigned char *) &data[offset], l, message);
    DLOG(INFO) << "send " << message;
    return l; // NOLINT(cppcoreguidelines-narrowing-conversions)
}

int serialportUnix::_read(unsigned char *data, int offset, int data_size, int timeout) {
    std::unique_lock<std::mutex> _lock{lock_wr};
    if (!_isOpened()) {
        return -1;
    }
    if (offset < 0) {
        LOG(WARNING) << "offset is " << offset;
        return -1;
    }
    int count = 0;
    unsigned char str[1] = {0};
    ssize_t wCount;
    size_t start = 0, stop = 0;
    start = logger::get_time_tick();
    stop = start + timeout;
    while (count < data_size) {
#ifdef ENABLE_POLL
        struct pollfd pfd[2]{};
        pfd[0].fd = hCom;
        pfd[0].events = POLLIN | POLLPRI |
                        POLLRDNORM |
                        POLLRDBAND;
        if (poll(pfd, 1, timeout) < 0 && !pfd[0].revents)
            break;
#endif
        wCount = ::read(hCom, str, 1);
        if (wCount <= 0) {
            start = logger::get_time_tick();
            if (stop > start && count <= 0) {
                continue;
            }
            break;
        } else {
            data[offset + count] = str[0];
            count++;
        }
    }
    return count;
}

