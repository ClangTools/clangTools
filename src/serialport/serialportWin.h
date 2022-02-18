//
// Created by caesar kekxv on 2021/4/12.
//

#ifndef TERMINALSERVICE_SERIALPORTWIN_H
#define TERMINALSERVICE_SERIALPORTWIN_H
#ifdef WIN32
#include "serialport.h"
#include <windows.h>
#include "easylogging++.h"

class serialportWin : public serialport {
private:
    HANDLE hCom = INVALID_HANDLE_VALUE;
    DCB dcb{};

    /**
     * 读取数据
     * @param data 读取数据内容
     * @param data_size 读取数据内容最大长度
     * @param timeout 超时时间
     * @return 读取数据内容长度
     */
    int _read(unsigned char *data, int offset, int data_size, int timeout) override;

    /**
     * 串口是否已经打开
     * @return 串口是否已经打开
     */
    bool _isOpened() override;

    void printf_dcb();

public:
    serialportWin();

    ~serialportWin() override;

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

#endif
#endif //TERMINALSERVICE_SERIALPORTWIN_H
