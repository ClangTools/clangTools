#pragma once

#include <cstdio>
#ifdef WIN32
#include <Windows.h>
#else
#include <ctime>
#include <termios.h>
#endif
#include <string>
#include <vector>

using namespace std;

#pragma region 宏定义
#ifndef DTR_CONTROL_HANDSHAKE
#define DTR_CONTROL_HANDSHAKE 0x02
#endif // !DTR_CONTROL_HANDSHAKE
#ifndef RTS_CONTROL_HANDSHAKE
#define RTS_CONTROL_HANDSHAKE 0x02
#endif // !RTS_CONTROL_HANDSHAKE
#ifndef NOPARITY
#define NOPARITY 0x02
#endif // !NOPARITY
#ifndef ONESTOPBIT
#define ONESTOPBIT 0x02
#endif // !ONESTOPBIT
#pragma endregion

typedef void(*ReadCallback)(int type, unsigned char data[], int size, void *that);

class SerialPort {
public:
    enum ErrorCode {
        Fail = -99,
        SendFail,
        ReadFail,
        TimeOutFail,
        CRCFail,
        OK = 0,
    };


    SerialPort();

    ~SerialPort();

    bool IsOpen();

    void SetReadCallback(ReadCallback cb, void *that);

    bool InitCom(const char *PortNum);

#ifdef WIN32

    static int GetList(int sList[], int len);

    /// <summary>
    /// 初始化
    /// </summary>
    int InitNum(int PortNum) {
        string com = R"(\\.\COM)" + to_string(PortNum);
        return InitCom(com.c_str());
    };
#else
    enum BaudRate {
        BR0 = 0000000,
        BR50 = 0000001,
        BR75 = 0000002,
        BR110 = 0000003,
        BR134 = 0000004,
        BR150 = 0000005,
        BR200 = 0000006,
        BR300 = 0000007,
        BR600 = 0000010,
        BR1200 = 0000011,
        BR1800 = 0000012,
        BR2400 = 0000013,
        BR4800 = 0000014,
        BR9600 = 0000015,
        BR19200 = 0000016,
        BR38400 = 0000017,
        BR57600 = 0010001,
        BR115200 = 0010002,
        BR230400 = 0010003,
        BR460800 = 0010004,
        BR500000 = 0010005,
        BR576000 = 0010006,
        BR921600 = 0010007,
        BR1000000 = 0010010,
        BR1152000 = 0010011,
        BR1500000 = 0010012,
        BR2000000 = 0010013,
        BR2500000 = 0010014,
        BR3000000 = 0010015,
        BR3500000 = 0010016,
        BR4000000 = 0010017
    };

    enum DataBits {
        DataBits5,
        DataBits6,
        DataBits7,
        DataBits8,
    };

    enum StopBits {
        StopBits1,
        StopBits2
    };

    enum Parity {
        ParityNone,
        ParityEven,
        PariteMark,
        ParityOdd,
        ParitySpace
    };

    struct OpenOptions {
        bool autoOpen;
        BaudRate baudRate;
        DataBits dataBits;
        StopBits stopBits;
        Parity parity;
        bool xon;
        bool xoff;
        bool xany;
        int vmin;
        int vtime;
    };
    static BaudRate BaudRateMake(unsigned long baudrate);
    static const OpenOptions defaultOptions;
    bool open(const std::string& path, const OpenOptions& options);
    static std::vector<std::string > list();
#endif

    void Free();
#ifdef WIN32
    void SetRtsControl(unsigned char Control = RTS_CONTROL_HANDSHAKE);

    void SetDtrControl(unsigned char Control = DTR_CONTROL_HANDSHAKE);

    void SetEvtEofChar(unsigned char EvtChar, unsigned char rEofChar);

    void SetBaudRate(unsigned long Baudtate = 115200);

    void SetByteSize(unsigned char byteSize = 8);

    void SetParity(unsigned char parity = NOPARITY);

    void SetStopBits(unsigned char stopBits = ONESTOPBIT);
#else
    void SetRtsControl(unsigned char Control = RTS_CONTROL_HANDSHAKE);

    void SetDtrControl(unsigned char Control = DTR_CONTROL_HANDSHAKE);

    void SetEvtEofChar(unsigned char EvtChar, unsigned char rEofChar);

    void SetBaudRate(SerialPort::BaudRate Baudtate = BR115200);

    void SetByteSize(SerialPort::DataBits _byteSize = SerialPort::DataBits8);

    void SetParity(SerialPort::Parity _parity = SerialPort::ParityNone);

    void SetStopBits(SerialPort::StopBits _stopBits = SerialPort::StopBits1);
#endif

public:
    //HANDLE hMutex1 = NULL;
#ifdef WIN32
    ///
#else
    void termiosOptions(termios& tios, const OpenOptions& options);
#endif

    int send(unsigned char data[], unsigned long len, unsigned long offset = 0);

    int read(int timeOut, unsigned char data[], int len);

    bool ClearCom();

    ReadCallback readCallback = nullptr;
    void *readCallbackThat = nullptr;

private:
#ifdef __FILENAME__
    const char *TAG = __FILENAME__;
#else
    const char *TAG = "SerialPort";
#endif

    //HANDLE hMutex1 = NULL;
#ifdef WIN32
    ///
    HANDLE hCom = nullptr;
#else
    int hCom{};
    std::string _path;
    OpenOptions _open_options{};
    int _tty_fd{};
    bool _is_open{};
#endif

    int EvtChar = -1;           //每个字节有8位
    int EofChar = -1;           //每个字节有8位

    unsigned char fRtsControl = RTS_CONTROL_HANDSHAKE;           //每个字节有8位
    unsigned char fDtrControl = DTR_CONTROL_HANDSHAKE;           //每个字节有8位
#ifdef WIN32
    unsigned long baudtate = 115200;
    unsigned char byteSize = 8;           //每个字节有8位
    unsigned char parity = NOPARITY;// NOPARITY;	 //无校验位
    unsigned char stopBits = ONESTOPBIT;// ONESTOPBIT; //1个停止位
#else
    SerialPort::BaudRate baudtate = SerialPort::BaudRate::BR115200;
    SerialPort::DataBits byteSize = SerialPort::DataBits::DataBits8;           //每个字节有8位
    SerialPort::Parity parity =SerialPort::Parity::ParityNone;
    SerialPort::StopBits stopBits =SerialPort::StopBits::StopBits1;
#endif
};

#ifndef WIN32

bool operator==(const SerialPort::OpenOptions& lhs, const SerialPort::OpenOptions& rhs);
bool operator!=(const SerialPort::OpenOptions& lhs, const SerialPort::OpenOptions& rhs);
#endif // WIN32

