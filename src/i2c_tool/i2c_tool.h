//
// Created by caesar kekxv on 2020/3/5.
//

#ifndef THERMTEMPIMAGE_I2C_TOOL_H
#define THERMTEMPIMAGE_I2C_TOOL_H

#include <vector>
#include <string>

class i2c_tool {
public:
    enum Type {
        Fail = -1,
        FailOpen = -2,
        FailAddr = -3,
        FailNotOpen = -3,
        FailRead = -4,
        Timeout = -10,
    };

    /**
     * 构造对象
     */
    i2c_tool();

    /**
     * 释放对象
     */
    ~i2c_tool();

    /**
     * 设置 iic(i2c) 地址
     * @param addr
     * @return
     */
    void setPath(const char *path = "/dev/i2c-1");

    /**
     * 设置 iic(i2c) 设备地址
     * @param addr
     * @return
     */
    unsigned char setAddr(unsigned char addr = 0x1A);

    /**
     * 打开设备
     * @return
     */
    int Open();

    /**
     * 判断是否打开
     * @return
     */
    bool IsOpen();

    /**
     * 关闭设备
     * @return
     */
    int Close();

    int Read(std::vector<unsigned char> &data, int read_len);

    int Write(std::vector<unsigned char> data);

    int Write(unsigned char *data, int data_len);

    int transfer(std::vector<unsigned char> wData, std::vector<unsigned char> *rData = nullptr, int read_len = 0,
                 int timeout = 10);

private:
    unsigned char Addr = 0x1A;
    std::string i2c_path = "/dev/i2c-1";
    int i2c_handle = -1;
};


#endif //THERMTEMPIMAGE_I2C_TOOL_H
