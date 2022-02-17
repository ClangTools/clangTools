//
// Created by caesar kekxv on 2021/4/12.
//

#include "serialportWin.h"
#include "easylogging++.h"

using namespace std;

serialport *serialport::newSerialport() {
    return new serialportWin();
}

void serialport::deleteSerialport(serialport *&_serialport) {
    if (_serialport != nullptr) {
        (_serialport)->stop();
        (_serialport)->close();
        delete (serialportWin *) _serialport;
    }
    _serialport = nullptr;
}

serialportWin::serialportWin() {
    _init();
}

serialportWin::~serialportWin() = default;

bool serialportWin::open(const char *path) {
    if (path == nullptr) {
        return false;
    }
    string com = path;
    if (com.find("COM") == com.npos) {
        com = R"(\\.\COM)" + string(path);
    }
    close();
    std::unique_lock <std::mutex> lock{lock_wr};
    hCom = CreateFile(com.c_str(),                  //串口名，COM10及以上的串口名格式应为："\\\\.\\COM10"
                      GENERIC_READ | GENERIC_WRITE, //允许读或写
                      0,                            //独占方式
                      nullptr,
                      OPEN_EXISTING,                //打开而不是创建
                      0,                            //同步方式
                      nullptr);
    if (!_isOpened()) {
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

    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hCom, &dcb)) {
        LOG(WARNING) << "error getting comm state";
    }
    // dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;//fRtsControl;
    // dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;//fDtrControl;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;//fRtsControl;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;//fDtrControl;
    return true;
}

void serialportWin::printf_dcb() {
    DCB dcb{};
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hCom, &dcb)) {
        LOG(WARNING) << "error getting comm state";
    } else {
        LOG(INFO) << "BaudRate : " << (DWORD) dcb.BaudRate;
        LOG(INFO) << "fBinary : " << (DWORD) dcb.fBinary;
        LOG(INFO) << "fParity : " << (DWORD) dcb.fParity;
        LOG(INFO) << "fOutxCtsFlow : " << (DWORD) dcb.fOutxCtsFlow;
        LOG(INFO) << "fOutxDsrFlow : " << (DWORD) dcb.fOutxDsrFlow;
        LOG(INFO) << "fDtrControl : " << (DWORD) dcb.fDtrControl;
        LOG(INFO) << "fDsrSensitivity : " << (DWORD) dcb.fDsrSensitivity;
        LOG(INFO) << "fTXContinueOnXoff : " << (DWORD) dcb.fTXContinueOnXoff;
        LOG(INFO) << "fOutX : " << (DWORD) dcb.fOutX;
        LOG(INFO) << "fInX : " << (DWORD) dcb.fInX;
        LOG(INFO) << "fErrorChar : " << (DWORD) dcb.fErrorChar;
        LOG(INFO) << "fNull : " << (DWORD) dcb.fNull;
        LOG(INFO) << "fRtsControl : " << (DWORD) dcb.fRtsControl;
        LOG(INFO) << "fAbortOnError : " << (DWORD) dcb.fAbortOnError;
        LOG(INFO) << "fDummy2 : " << (DWORD) dcb.fDummy2;
        LOG(INFO) << "wReserved : " << (WORD) dcb.wReserved;
        LOG(INFO) << "XonLim : " << (WORD) dcb.XonLim;
        LOG(INFO) << "XoffLim : " << (WORD) dcb.XoffLim;
        LOG(INFO) << "ByteSize : " << (int) dcb.ByteSize;
        LOG(INFO) << "Parity : " << (int) dcb.Parity;
        LOG(INFO) << "StopBits : " << (int) dcb.StopBits;
        LOG(INFO) << "XonChar : " << (int) dcb.XonChar;
        LOG(INFO) << "XoffChar : " << (int) dcb.XoffChar;
        LOG(INFO) << "ErrorChar : " << (int) dcb.ErrorChar;
        LOG(INFO) << "EofChar : " << (int) dcb.EofChar;
        LOG(INFO) << "EvtChar : " << (int) dcb.EvtChar;
        LOG(INFO) << "wReserved1 : " << (WORD) dcb.wReserved1;
    }
}

bool serialportWin::close() {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return true;
    CloseHandle(hCom);
    hCom = INVALID_HANDLE_VALUE;
    return true;
}

bool serialportWin::_isOpened() {
    return hCom != INVALID_HANDLE_VALUE;
}

/**
 * 设置波特率
 * @param baud_rate 波特率
 * @return 当前 serialport 对象
 */
serialport *serialportWin::set_baud_rate(BaudRate baud_rate) {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return this;
    unsigned long _baud_rate = 9600;
    switch (baud_rate) {
        case BaudRate::BR50:
            _baud_rate = 50;
            break;
        case BaudRate::BR75:
            _baud_rate = 75;
            break;
        case BaudRate::BR134:
            _baud_rate = 134;
            break;
        case BaudRate::BR150:
            _baud_rate = 150;
            break;
        case BaudRate::BR200:
            _baud_rate = 200;
            break;
        case BaudRate::BR300:
            _baud_rate = 300;
            break;
        case BaudRate::BR600:
            _baud_rate = 600;
            break;
        case BaudRate::BR1200:
            _baud_rate = 1200;
            break;
        case BaudRate::BR1800:
            _baud_rate = 1800;
            break;
        case BaudRate::BR2400:
            _baud_rate = 2400;
            break;
        case BaudRate::BR4800:
            _baud_rate = 4800;
            break;
        case BaudRate::BR9600:
            _baud_rate = 9600;
            break;
        case BaudRate::BR19200:
            _baud_rate = 19200;
            break;
        case BaudRate::BR38400:
            _baud_rate = 38400;
            break;
        case BaudRate::BR57600:
            _baud_rate = 57600;
            break;
        case BaudRate::BR115200:
            _baud_rate = 115200;
            break;
        case BaudRate::BR230400:
            _baud_rate = 230400;
            break;
        case BaudRate::BR460800:
            _baud_rate = 460800;
            break;
        case BaudRate::BR500000:
            _baud_rate = 500000;
            break;
        case BaudRate::BR576000:
            _baud_rate = 576000;
            break;
        case BaudRate::BR921600:
            _baud_rate = 921600;
            break;
        case BaudRate::BR1000000:
            _baud_rate = 1000000;
            break;
        case BaudRate::BR1152000:
            _baud_rate = 1152000;
            break;
        case BaudRate::BR1500000:
            _baud_rate = 1500000;
            break;
        case BaudRate::BR2000000:
            _baud_rate = 2000000;
            break;
        case BaudRate::BR2500000:
            _baud_rate = 2500000;
            break;
        case BaudRate::BR3000000:
            _baud_rate = 3000000;
            break;
        case BaudRate::BR3500000:
            _baud_rate = 3500000;
            break;
        case BaudRate::BR4000000:
            _baud_rate = 4000000;
            break;
        default:
            break;
    }
    if (!GetCommState(hCom, &dcb)) {
        LOG(WARNING) << "GetCommState : false";
        return this;
    }
    LOG(INFO) << "BaudRate " << dcb.BaudRate;
    dcb.BaudRate = _baud_rate;     //波特率为9600
    BOOL flag = SetCommState(hCom, &dcb);
    LOG(INFO) << (flag ? "true" : "false");
    return this;
}

/**
 * 设置数据位数
 * @param data_bits
 * @return 当前 serialport 对象
 */
serialport *serialportWin::set_data_bits(DataBits data_bits) {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return this;
    if (!GetCommState(hCom, &dcb)) {
        LOG(WARNING) << "GetCommState : false";
        return this;
    }
    LOG(INFO) << "ByteSize " << dcb.ByteSize;
    dcb.ByteSize = (unsigned char) data_bits;
    BOOL flag = SetCommState(hCom, &dcb);
    LOG(INFO) << (flag ? "true" : "false");
    return this;
}

/**
 * 设置停止位
 * @param stop_bits
 * @return 当前 serialport 对象
 */
serialport *serialportWin::set_stop_bits(StopBits stop_bits) {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return this;
    if (!GetCommState(hCom, &dcb)) {
        LOG(WARNING) << "GetCommState : false";
        return this;
    }
    LOG(INFO) << "StopBits " << dcb.StopBits;
    switch (stop_bits) {
        case StopBits1:
            dcb.StopBits = ONESTOPBIT;
            break;
        case StopBits1_5:
            dcb.StopBits = ONE5STOPBITS;
            break;
        case StopBits2:
            dcb.StopBits = TWOSTOPBITS;
            break;
    }
    BOOL flag = SetCommState(hCom, &dcb);
    LOG(INFO) << (flag ? "true" : "false");
    return this;
}

/**
 * 设置校验位
 * @param parity
 * @return 当前 serialport 对象
 */
serialport *serialportWin::set_parity(Parity parity) {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return this;
    if (!GetCommState(hCom, &dcb)) {
        LOG(WARNING) << "GetCommState : false";
        return this;
    }
    LOG(INFO) << "Parity " << dcb.Parity;
    switch (parity) {
        case ParityNone:
            dcb.Parity = NOPARITY;
            break;
        case ParityOdd:
            dcb.Parity = ODDPARITY;
            break;
        case ParityEven:
            dcb.Parity = EVENPARITY;
            break;
        case ParityMark:
            dcb.Parity = MARKPARITY;
            break;
        case ParitySpace:
            dcb.Parity = SPACEPARITY;
            break;
        default:
            dcb.Parity = NOPARITY;
            break;
    }
    BOOL flag = SetCommState(hCom, &dcb);
    LOG(INFO) << (flag ? "true" : "false");
    return this;
}

/**
 * 设置 XON XOFF XANY
 * @param XON
 * @param XOFF
 * @param XANY
 * @return
 */
serialport *serialportWin::set_XON_OFF_ANY(int XON, int XOFF, int XANY) {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return this;
    if (!GetCommState(hCom, &dcb)) {
        LOG(WARNING) << "GetCommState : false";
        return this;
    }
    dcb.fRtsControl = (XANY >> 4) & 0xf;//fRtsControl;
    dcb.fDtrControl = XANY & 0xf;//fDtrControl;
    // if (EofChar > 0) {
    if (XON > 0) {
        // dcb.EofChar = EofChar & 0xFF;         /* End of Input character          */
        dcb.EofChar = XON & 0xFF;         /* End of Input character          */
    }
    // if (EvtChar > 0) {
    if (XOFF > 0) {
        // dcb.EvtChar = EvtChar & 0xFF;         /* Received Event character        */
        dcb.EvtChar = XOFF & 0xFF;         /* Received Event character        */
    }
    SetCommState(hCom, &dcb);
    return this;
}

int serialportWin::send(const unsigned char *data, int offset, int data_size, int timeout) {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return -1;
    DWORD len = 0;
    int ret = WriteFile(hCom, &data[offset], data_size, &len, nullptr);
    if (!ret) {
        LOG(ERROR) << "WriteFile data fail " << GetLastError();
        return -2;
    }
    // if (len > 0) {
    //     std::string message;
    //     tools::bytes_to_hex_string((unsigned char *) &data[offset], len, message);
    //     DLOG(INFO) << "send " << message;
    // }
    return len;
}

int serialportWin::_read(unsigned char *data, int offset, int data_size, int timeout) {
    std::unique_lock <std::mutex> lock{lock_wr};
    if (!_isOpened())return -1;
    int count = 0;
    bool bReadStat;
    unsigned char str[1] = {0};
    DWORD wCount; //读取的字节数
    size_t start = 0, stop = 0;
    COMMTIMEOUTS TimeOuts;
    GetCommTimeouts(hCom, &TimeOuts); //设置超时
    //设定读超时
    TimeOuts.ReadIntervalTimeout = 50;
    TimeOuts.ReadTotalTimeoutMultiplier = 30;
    TimeOuts.ReadTotalTimeoutConstant = 50;
    SetCommTimeouts(hCom, &TimeOuts); //设置超时
    start = GetTickCount();
    stop = start + timeout;
    while (count < data_size) {
        bReadStat = ReadFile(hCom, str, 1, &wCount, nullptr);
        if (!bReadStat) {
            break;
        } else {
            if (wCount <= 0) {
                start = GetTickCount();
                if (stop > start && count <= 0) {
                    continue;
                }
                break;
            } else {
                data[offset + count] = str[0];
                count++;
            }
        }
    }
    return count;
}
