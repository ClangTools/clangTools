//
// Created by caesar kekxv on 2020/8/5.
//

#ifndef TOOLS_PRINTERTOOLREADWRITEAGENT_H
#define TOOLS_PRINTERTOOLREADWRITEAGENT_H

#include <vector>

class _DLL_ClangTools_Export PrinterToolReadWriteAgent {
public:
    enum Error {
        Error_OK = 0,
        Error_Fail = -1,
        Error_SendFail = -2,
        Error_ReadFail = -3,
        Error_Timeout = -4,
        Error_NotOpen = -9,
    };

    /**
     * 发送数据
     * @param data
     * @param offset offset 发送数据长度位移，默认为 0
     * @return 返回发送的数据长度
     */
    int Send(std::vector<unsigned char> data, int offset);

    /**
     * 发送数据
     * @param data
     * @param len 需要发送的数据长度
     * @return 返回发送的数据长度
     */
    virtual int Send(unsigned char *data, int len);
    /**
     * 发送数据
     * @param data
     * @param len 需要发送的数据长度
     * @param offset 发送数据长度位移，默认为 0
     * @return 返回发送的数据长度
     */
    virtual int Send(unsigned char *data, int len, int offset) = 0;

    /**
     * 读取数据
     * @param data
     * @param readSize 需要读取的长度
     * @return 返回读取的数据长度
     */
    int Read(std::vector<unsigned char> &data, int readSize);

    /**
     * 读取数据
     * @param data
     * @param dataSize 数据长度
     * @param offset 读取的位移
     * @return 返回读取的数据长度
     */
    virtual int Read(unsigned char *data, int dataSize, int offset) = 0;

    /**
     * 读取数据
     * @param data
     * @param dataSize 数据长度
     * @return 返回读取的数据长度
     */
    virtual int Read(unsigned char *data, int dataSize);
};


#endif //TOOLS_PRINTERTOOLREADWRITEAGENT_H
