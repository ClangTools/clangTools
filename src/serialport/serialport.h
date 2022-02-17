//
// Created by caesar kekxv on 2021/4/11.
//

#ifndef TERMINALSERVICE_SERIALPORT_H
#define TERMINALSERVICE_SERIALPORT_H

#include "thread_pool.hpp"
#include "mutex"

class serialport {
public:
    typedef void(*read_callback_func)(unsigned char data[], int size, void *that);

    /**
     * 创建串口设备
     * @return
     */
    static serialport *newSerialport();

    /**
     * 释放串口设备
     * @return
     */
    static void deleteSerialport(serialport *&_serialport);

    /**
     * 波特率
     */
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

    /**
     * 数据位
     */
    enum DataBits {
        DataBits5 = 5,
        DataBits6 = 6,
        DataBits7 = 7,
        DataBits8 = 8,
    };

    /**
     * 停止位
     */
    enum StopBits {
        StopBits1 = 1,
        StopBits2 = 2,
        StopBits1_5 = 3,
    };

    /**
     * 校验位
     */
    enum Parity {
        ParityNone = 'N',
        ParityEven = 'E',
        ParityMark = 'M',
        ParityOdd = 'O',
        ParitySpace = 'S'
    };

protected:
    /**
     * 读线程
     */
    thread_pool pool{1};

    /**
     * 是否退出释放
     */
    std::atomic<bool> stopped{true};

    /**
     * 是否退出释放完成
     */
    std::atomic<bool> stop_finish{false};

    /**
     * 读写线程锁
     */
    std::mutex lock_wr{};

    /**
     * 操作 read_data 锁
     */
    std::mutex lock{};

    std::condition_variable stop_cv;
    /**
     * 回调函数
     */
    read_callback_func readCallbackFunc = nullptr;

    /**
     * 回调上下文
     */
    void *that = nullptr;

    /**
     * 串口读缓存最大大小
     * 1M
     */
    unsigned long long read_data_size = 1024 * 1024;

    /**
     * 串口读缓存
     */
    std::vector<unsigned char> read_data;

    /**
     * 插入数据
     * @param data
     */
    void push_back(unsigned char data);

    bool _init();

    /**
     * 读取数据
     * @param data 读取数据内容
     * @param data_size 读取数据内容最大长度
     * @param timeout 超时时间
     * @return 读取数据内容长度
     */
    virtual int _read(unsigned char *data, int offset, int data_size, int timeout) = 0;

    /**
     * 串口是否已经打开
     * @return 串口是否已经打开
     */
    virtual bool _isOpened() = 0;

    void stop();


public:

    /**
     * 初始化
     */
    explicit serialport(read_callback_func cb = nullptr, void *that = nullptr);

    /**
     * 释放
     */
    virtual ~serialport() = 0;

    /**
     * 开启串口
     * @param path  串口地址
     * @return 是否开启成功
     */
    virtual bool open(const char *path) = 0;

    /**
     * 关闭串口
     * @return 是否关闭成功
     */
    virtual bool close() = 0;

    /**
     * 串口是否已经打开
     * @return 串口是否已经打开
     */
    bool isOpened();

    /**
     * 设置波特率
     * @param baud_rate 波特率
     * @return 当前 serialport 对象
     */
    virtual serialport *set_baud_rate(BaudRate baud_rate) = 0;

    /**
     * 设置数据位数
     * @param data_bits
     * @return 当前 serialport 对象
     */
    virtual serialport *set_data_bits(DataBits data_bits) = 0;

    /**
     * 设置停止位
     * @param stop_bits
     * @return 当前 serialport 对象
     */
    virtual serialport *set_stop_bits(StopBits stop_bits) = 0;

    /**
     * 设置校验位
     * @param parity
     * @return 当前 serialport 对象
     */
    virtual serialport *set_parity(Parity parity) = 0;

    /**
     * 设置 XON XOFF XANY
     * @param XON
     * @param XOFF
     * @param XANY
     * @return
     */
    virtual serialport *set_XON_OFF_ANY(int XON, int XOFF, int XANY);


    /**
     * 发送数据
     * @param data 发送数据内容
     * @param timeout 超时时间
     * @return 发送成功数据长度
     */
    int send(const std::vector<unsigned char> &data, int timeout = 300);

    /**
     * 发送数据
     * @param data 发送数据内容
     * @param data_size 发送数据内容长度
     * @param timeout 超时时间
     * @return 发送成功数据长度
     */
    virtual int send(const unsigned char *data, int offset, int data_size, int timeout) = 0;

    /**
     * 读取数据
     * @param data 读取数据内容
     * @param data_size 读取数据内容最大长度
     * @param timeout 超时时间
     * @return 读取数据内容长度
     */
    int read(std::vector<unsigned char> &data, int data_size, int timeout = 300);

    /**
     * 读取数据
     * @param data 读取数据内容
     * @param data_size 读取数据内容最大长度
     * @param timeout 超时时间
     * @return 读取数据内容长度
     */
    int read(unsigned char *data, int offset, int data_size, int timeout = 300);

    /**
     * 可用数据长度
     * @return 可用数据长度
     */
    int available();

    /**
     * 清理读取数据
     * @return 清理掉的数据长度
     */
    int clean();
};


#endif //TERMINALSERVICE_SERIALPORT_H
