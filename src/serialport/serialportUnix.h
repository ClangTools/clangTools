//
// Created by caesar kekxv on 2021/4/12.
//

#ifndef TERMINALSERVICE_SERIALPORTUNIX_H
#define TERMINALSERVICE_SERIALPORTUNIX_H


#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <termios.h>
#include "serialport.h"

class serialportUnix : public serialport {
private:
    int hCom = -1;

    /**
     * 开启之前的配置记录
     */
    struct termios old_tio{};

    /**
     * 开启之前的配置记录记录状态
     */
    bool old_tio_flag = true;

    /**
     * 读取数据
     * @param data 读取数据内容
     * @param data_size 读取数据内容最大长度
     * @param timeout 超时时间
     * @return 读取数据内容长度
     */
    int _read(unsigned char *data, int offset, int data_size, int timeout) override;

public:
    serialportUnix();

    ~serialportUnix() override;

    /**
     * 开启串口
     * @param path  串口地址
     * @return 是否开启成功
     */
    bool open(const char *path) override;

    /**
     * 关闭串口
     * @return 是否关闭成功
     */
    bool close() override;

    /**
     * 串口是否已经打开
     * @return 串口是否已经打开
     */
    bool _isOpened() override;

    /**
     * 设置波特率
     * @param baud_rate 波特率
     * @return 当前 serialport 对象
     */
    serialport *set_baud_rate(BaudRate baud_rate) override;

    /**
     * 设置数据位数
     * @param data_bits
     * @return 当前 serialport 对象
     */
    serialport *set_data_bits(DataBits data_bits) override;

    /**
     * 设置停止位
     * @param stop_bits
     * @return 当前 serialport 对象
     */
    serialport *set_stop_bits(StopBits stop_bits) override;

    /**
     * 设置校验位
     * @param parity
     * @return 当前 serialport 对象
     */
    serialport *set_parity(Parity parity) override;

    /**
     * 设置 XON XOFF XANY
     * @param XON
     * @param XOFF
     * @param XANY
     * @return
     */
    serialport *set_XON_OFF_ANY(int XON, int XOFF, int XANY) override;

    /**
     * 发送数据
     * @param data 发送数据内容
     * @param data_size 发送数据内容长度
     * @param timeout 超时时间
     * @return 发送成功数据长度
     */
    int send(const unsigned char *data, int offset, int data_size, int timeout) override;

};


#endif //TERMINALSERVICE_SERIALPORTUNIX_H
