#include "SerialPort.h"
#include <logger.h>

#ifdef WIN32
#else

#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <poll.h>

#endif
using namespace std;

SerialPort::SerialPort() = default;

SerialPort::~SerialPort() {
    Free();
    //if (hMutex1 != NULL) {
    //	CloseHandle(hMutex1);
    //	hMutex1 = NULL;
    //}
}


bool SerialPort::IsOpen() {
#ifdef WIN32
    return !(hCom == NULL || hCom == (HANDLE) -1);
#else
    return _is_open && hCom > 0;
#endif
}

#ifdef WIN32

int SerialPort::GetList(int sList[], int len) {
    int count = 0;
    for (int i = 0; i < 256; i++) {
        string SpName = R"(\\.\COM)" + to_string(i);
        HANDLE hCom = CreateFile(TEXT(SpName.c_str()),                //COM1口
                                 GENERIC_READ | GENERIC_WRITE, //允许读和写
                                 0,                            //独占方式
                                 nullptr,
                                 OPEN_EXISTING, //打开而不是创建
                                 0,             //同步方式
                                 nullptr);
        if (hCom != (HANDLE) -1 && hCom != nullptr) {
            CloseHandle(hCom);
            hCom = nullptr;
            sList[count++] = i;
            if (count > len) {
                break;
            }
        }
    }
    return count;
}

#endif

bool SerialPort::InitCom(const char *PortNum) {
#ifdef WIN32
    hCom = CreateFile(TEXT(PortNum),                //COM1口
                      GENERIC_READ | GENERIC_WRITE, //允许读和写
                      0,                            //独占方式
                      nullptr,
                      OPEN_EXISTING, //打开而不是创建
                      0,             //同步方式
                      nullptr);
    if (hCom == (HANDLE) -1) {
        return false;
    }
    SetupComm(hCom, 1024, 1024); //输入缓冲区和输出缓冲区的大小都是1024
    COMMTIMEOUTS TimeOuts;
    //设定读超时
    TimeOuts.ReadIntervalTimeout = 100;
    TimeOuts.ReadTotalTimeoutMultiplier = 50;
    TimeOuts.ReadTotalTimeoutConstant = 1000;
    //设定写超时
    TimeOuts.WriteTotalTimeoutMultiplier = 500;
    TimeOuts.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(hCom, &TimeOuts); //设置超时
    DCB dcb;
    GetCommState(hCom, &dcb);
    dcb.BaudRate = baudtate;     //波特率为9600
    dcb.ByteSize = byteSize;           //每个字节有8位
    dcb.Parity = parity;// NOPARITY;	 //无校验位
    dcb.StopBits = stopBits;// ONESTOPBIT; //1个停止位
    dcb.fRtsControl = fRtsControl;
    dcb.fDtrControl = fDtrControl;
    if (EofChar > 0) {
        dcb.EofChar = EofChar & 0xFF;         /* End of Input character          */
    }
    if (EvtChar > 0) {
        dcb.EvtChar = EvtChar & 0xFF;         /* Received Event character        */
    }
    SetCommState(hCom, &dcb);
#else
    if (_path != PortNum) _path = PortNum;

    SerialPort::OpenOptions options(SerialPort::defaultOptions);
    options.parity = parity;// SerialPort::Parity::ParityNone;
    options.dataBits = byteSize;// SerialPort::DataBits::DataBits8;
    options.stopBits = stopBits;// SerialPort::StopBits::StopBits1;
    options.baudRate = BaudRateMake(baudtate);

    _open_options = options;

    hCom = ::open(PortNum, O_RDWR | O_NOCTTY | O_NONBLOCK/* | O_SYNC*/);
    if (hCom < 0) {
        _is_open = false;
        return false;
    }
#if true
    fcntl(hCom, F_SETFL, O_RDWR);
    set_opt(hCom, baudtate, SerialPort::DataBits::DataBits8, SerialPort::Parity::ParityNone,
            SerialPort::StopBits::StopBits1);
#endif
#if false
    struct termios tios{};
    termiosOptions(tios, _open_options);
    tcsetattr(_tty_fd, TCSANOW, &tios);
    tcflush(_tty_fd, TCIOFLUSH);
#endif
    _is_open = true;
#endif
    return IsOpen();
}

void SerialPort::Free() {
    if (!IsOpen())return;
#ifdef WIN32
    CloseHandle(hCom);
    hCom = nullptr;
#else
    if (old_flag)
        if ((tcsetattr(hCom, TCSAFLUSH, &oldtio)) != 0) {
            logger::instance()->e(TAG, __LINE__, "SetupSerial com set error %s", strerror(errno));
        }
    ::close(hCom);
    _is_open = false;
#endif
}

#ifdef WIN32

void SerialPort::SetBaudRate(unsigned long Baudtate) {
    baudtate = Baudtate;
}

void SerialPort::SetByteSize(unsigned char byteSize) {
    this->byteSize = byteSize;
}

void SerialPort::SetParity(unsigned char parity) {
    this->parity = parity;
}

void SerialPort::SetStopBits(unsigned char stopBits) {
    this->stopBits = stopBits;
}

#else

void SerialPort::SetBaudRate(SerialPort::BaudRate Baudtate) {
    baudtate = Baudtate;
}

void SerialPort::SetByteSize(SerialPort::DataBits _byteSize) {
    this->byteSize = _byteSize;
}

void SerialPort::SetParity(SerialPort::Parity _parity) {
    this->parity = _parity;
}

void SerialPort::SetStopBits(SerialPort::StopBits _stopBits) {
    this->stopBits = _stopBits;
}

#endif

void SerialPort::SetRtsControl(unsigned char Control) {
    this->fRtsControl = Control;
}

void SerialPort::SetDtrControl(unsigned char Control) {
    this->fDtrControl = Control;
}

void SerialPort::SetEvtEofChar(unsigned char EvtChar, unsigned char EofChar) {
    this->EvtChar = EvtChar;
    this->EofChar = EofChar;
}


int SerialPort::send(unsigned char data[], unsigned long len, unsigned long offset) {
    if (!IsOpen()) {
        return Fail;
    }
#ifdef WIN32
    //WaitForSingleObject(hMutex1, INFINITE);
    int ret = WriteFile(hCom, data, len, &len, nullptr) ? OK : SendFail;
    // logger::instance()->puts_info((string(TAG) + ":" + to_string(__LINE__)).c_str(), "send", data, len);
    //ReleaseMutex(hMutex1);
    return ret;
#else
    size_t l = ::write(hCom, &data[offset], len);
    return len == l ? OK : SendFail;
#endif
}

int SerialPort::read(int timeOut, unsigned char data[], int len) {
    if (!IsOpen()) {
        return Fail;
    }
    //WaitForSingleObject(hMutex1, INFINITE);
    int count = 0;
    bool bReadStat;
    //PurgeComm(hCom,PURGE_RXCLEAR); //清空缓冲区
    unsigned char str[1] = {0};
#ifdef WIN32
    DWORD wCount; //读取的字节数
#else
    ssize_t wCount;
#endif
    size_t start = 0, stop = 0;
#ifdef WIN32
    COMMTIMEOUTS TimeOuts;
    //设定读超时
    TimeOuts.ReadIntervalTimeout = 100;
    TimeOuts.ReadTotalTimeoutMultiplier = 50;
    TimeOuts.ReadTotalTimeoutConstant = 100;
    //设定写超时
    TimeOuts.WriteTotalTimeoutMultiplier = 100;
    TimeOuts.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(hCom, &TimeOuts); //设置超时
    start = GetTickCount();
#else
    start = logger::get_time_tick();
#endif

    stop = start + timeOut;

    // logger::instance()->d(TAG, __LINE__, "read start %d ", timeOut);
    while (count < len) {
#ifdef WIN32
        bReadStat = ReadFile(hCom, str, 1, &wCount, nullptr);
        if (!bReadStat) {
            break;
        } else {
#else
#ifdef ENABLE_POLL
        struct pollfd pfd[2]{};
        pfd[0].fd = hCom;
        pfd[0].events = POLLIN | POLLPRI |
                        POLLRDNORM |
                        POLLRDBAND;
        if (poll(pfd, 1, timeOut) < 0 && !pfd[0].revents)
            break;
#endif
        wCount = ::read(hCom, str, 1);
#endif
        if (wCount <= 0) {
#ifdef WIN32
            start = GetTickCount();
#else
            start = logger::get_time_tick();
#endif
            if (stop > start && count <= 0) {
                continue;
            }
            break;
        } else {
            data[count] = str[0];
            count++;
        }
#ifdef WIN32
        }
#endif
    }
    // logger::instance()->d(TAG, __LINE__, "read end");
    //ReleaseMutex(hMutex1);
    // logger::instance()->puts_info((string(TAG) + ":" + to_string(__LINE__)).c_str(), "Read", data, count);
    return count;
}

void SerialPort::SetReadCallback(ReadCallback cb, void *that) {
    readCallback = cb;
    readCallbackThat = that;
}

bool SerialPort::ClearCom() {
    if (!IsOpen())return false;
#ifdef WIN32
    return (bool) PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
#else
    return tcflush(hCom, TCIOFLUSH);
#endif
}

#ifndef WIN32

void SerialPort::termiosOptions(termios &tios, const OpenOptions &options) {
    tcgetattr(_tty_fd, &tios);
    tios.c_oflag = 0;
    tios.c_iflag = 0;
    tios.c_lflag = 0;
    cfsetispeed(&tios, options.baudRate);
    cfsetospeed(&tios, options.baudRate);
    tios.c_iflag |= (options.xon ? IXON : 0)
                    | (options.xoff ? IXOFF : 0)
                    | (options.xany ? IXANY : 0);

    // data bits

    int databits[] = {CS5, CS6, CS7, CS8};
    tios.c_cflag &= ~0x30;
    tios.c_cflag |= databits[options.dataBits];

    // stop bits
    if (options.stopBits == StopBits2) {
        tios.c_cflag |= CSTOPB;
    } else {
        tios.c_cflag &= ~CSTOPB;
    }

    // parity
    if (options.parity == ParityNone) {
        tios.c_cflag &= ~PARENB;
    } else {
        tios.c_cflag |= PARENB;

        if (options.parity == PariteMark) {
            tios.c_cflag |= PARMRK;
        } else {
            tios.c_cflag &= ~PARMRK;
        }

        if (options.parity == ParityOdd) {
            tios.c_cflag |= PARODD;
        } else {
            tios.c_cflag &= ~PARODD;
        }
    }

    tios.c_cc[VMIN] = options.vmin;
    tios.c_cc[VTIME] = options.vtime;
}

const SerialPort::OpenOptions SerialPort::defaultOptions = {
        false, //        bool autoOpen;
        SerialPort::BR9600, //    BaudRate baudRate;
        SerialPort::DataBits8, //    DataBits dataBits;
        SerialPort::StopBits1, //    StopBits stopBits;
        SerialPort::ParityNone,//    Parity parity;
        false,                  // input xon
        false,                  // input xoff
        false,                  // input xany
        0,                      // c_cc vmin
        50,                     // c_cc vtime
};

unsigned long SerialPort::BaudRateMake(unsigned long baudrate) {
    switch (baudrate) {
        case 50:
            return B50;
        case 75:
            return B75;
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
            break;
    }
    return B0;
}


std::vector<std::string> SerialPort::list() {
    DIR *dir;
    struct dirent *ent;
    dir = opendir("/dev");
    std::vector<std::string> ttyList;

    while (ent = readdir(dir), ent != nullptr) {
        string name = std::string(ent->d_name);
        if ("tty" == name.substr(0, 3) && name.size() > 3 &&
            (name[3] > '9' /*&& (name[3] != 'S' && name[3] != 'T')*/)) {
            // logger::instance()->d(TAG, __LINE__, "%s", ent->d_name);
            ttyList.emplace_back(ent->d_name);
        }
    }

    return ttyList;
}

bool SerialPort::open(const std::string &path, const SerialPort::OpenOptions &options) {
    _open_options = options;

    hCom = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (hCom < 0) {
        _is_open = false;
        return false;
    }

    struct termios tios{};
    termiosOptions(tios, _open_options);
    tcsetattr(_tty_fd, TCSANOW, &tios);
    tcflush(_tty_fd, TCIOFLUSH);
    _is_open = true;
    return _is_open;
}

int SerialPort::set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop) {
    struct termios newtio{};
    if (tcgetattr(fd, &oldtio) != 0) {
        logger::instance()->e(TAG, __LINE__, "SetupSerial %s", strerror(errno));
        return -1;
    }
    old_flag = true;
    // bzero(&newtio, sizeof(newtio));
    cfmakeraw(&newtio);
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch (nBits) {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
        default:
            newtio.c_cflag |= CS8;
            break;
    }

    switch (nEvent) {
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E':
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        default:
        case 'N':
            newtio.c_cflag &= ~PARENB;
            break;
    }

    cfsetspeed(&newtio, BaudRateMake(nSpeed));

    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    // tcflush(fd, TCIFLUSH);
    if ((tcsetattr(fd, TCSAFLUSH, &newtio)) != 0) {
        logger::instance()->e(TAG, __LINE__, "SetupSerial com set error %s", strerror(errno));
        return -1;
    }
    // printf("set done!\n\r");
    return 0;
}


bool operator==(const SerialPort::OpenOptions &lhs, const SerialPort::OpenOptions &rhs) {
    return lhs.autoOpen == rhs.autoOpen
           && lhs.baudRate == rhs.baudRate
           && lhs.dataBits == rhs.dataBits
           && lhs.parity == rhs.parity
           && lhs.stopBits == rhs.stopBits
           && lhs.vmin == rhs.vmin
           && lhs.vtime == rhs.vtime
           && lhs.xon == rhs.xon
           && lhs.xoff == rhs.xoff
           && lhs.xany == rhs.xany;
}

bool operator!=(const SerialPort::OpenOptions &lhs, const SerialPort::OpenOptions &rhs) {
    return !(lhs == rhs);
}

#endif

