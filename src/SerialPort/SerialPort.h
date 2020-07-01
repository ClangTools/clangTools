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

#pragma region define_value
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
    /// init
    /// </summary>
    int InitNum(int PortNum) {
        string com = R"(\\.\COM)" + to_string(PortNum);
        return InitCom(com.c_str());
    };
#else
    bool old_flag = false;
    struct termios oldtio{};
    int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);

    enum BaudRate {
        BR0 = 0,
        BR50 = 50,
        BR75 = 75,
        BR110 = 110,
        BR134 = 134,
        BR150 = 150,
        BR200 = 200,
        BR300 = 300,
        BR600 = 600,
        BR1200 = 1200,
        BR1800 = 1800,
        BR2400 = 2400,
        BR4800 = 4800,
        BR9600 = 9600,
        BR19200 = 19200,
        BR38400 = 38400,
        BR57600 = 57600,
        BR115200 = 115200,
        BR230400 = 230400,
        BR460800 = 460800,
        BR500000 = 500000,
        BR576000 = 576000,
        BR921600 = 921600,
        BR1000000 = 1000000,
        BR1152000 = 1152000,
        BR1500000 = 1500000,
        BR2000000 = 2000000,
        BR2500000 = 2500000,
        BR3000000 = 3000000,
        BR3500000 = 3500000,
        BR4000000 = 4000000
    };

    enum DataBits {
        DataBits5 = 5,
        DataBits6 = 6,
        DataBits7 = 7,
        DataBits8 = 8,
    };

    enum StopBits {
        StopBits1 = 1,
        StopBits2 = 2
    };

    enum Parity {
        ParityNone = 'N',
        ParityEven = 'E',
        PariteMark = 'M',
        ParityOdd = 'O',
        ParitySpace = 'S'
    };

    struct OpenOptions {
        bool autoOpen;
        unsigned long baudRate;
        DataBits dataBits;
        StopBits stopBits;
        Parity parity;
        bool xon;
        bool xoff;
        bool xany;
        int vmin;
        int vtime;
    };

    static unsigned long BaudRateMake(unsigned long baudrate);

    static const OpenOptions defaultOptions;

    bool open(const std::string &path, const OpenOptions &options);

    static std::vector<std::string> list();

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

    void termiosOptions(termios &tios, const OpenOptions &options);

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

    int EvtChar = -1;           //every one byte is 8
    int EofChar = -1;           //every one byte is 8

    unsigned char fRtsControl = RTS_CONTROL_HANDSHAKE;           //every one byte is 8
    unsigned char fDtrControl = DTR_CONTROL_HANDSHAKE;           //every one byte is 8
#ifdef WIN32
    unsigned long baudtate = 115200;
    unsigned char byteSize = 8;           //every one byte is 8
    unsigned char parity = NOPARITY;// NOPARITY;	 // no parity
    unsigned char stopBits = ONESTOPBIT;// ONESTOPBIT; //1 stop
#else
    SerialPort::BaudRate baudtate = SerialPort::BaudRate::BR115200;
    SerialPort::DataBits byteSize = SerialPort::DataBits::DataBits8;           //every one byte is 8
    SerialPort::Parity parity = SerialPort::Parity::ParityNone;
    SerialPort::StopBits stopBits = SerialPort::StopBits::StopBits1;
#endif
};

#ifndef WIN32

bool operator==(const SerialPort::OpenOptions &lhs, const SerialPort::OpenOptions &rhs);

bool operator!=(const SerialPort::OpenOptions &lhs, const SerialPort::OpenOptions &rhs);

#endif // WIN32

