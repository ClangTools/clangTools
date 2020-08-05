//
// Created by caesar kekxv on 2020/8/5.
//

#include "PrinterToolReadWriteAgent.h"

int PrinterToolReadWriteAgent::Send(std::vector<unsigned char> data, int offset) {
    return Send(data.data(), (int) data.size() - offset, offset);
}

int PrinterToolReadWriteAgent::Read(std::vector<unsigned char> &data, int readSize) {
    auto *rData = new unsigned char[readSize + 1];
    int ret = Read(rData, readSize, 0);
    if (ret > 0) {
        data.insert(data.end(), &rData[0], &rData[ret]);
    }
    delete[] rData;
    rData = nullptr;
    return ret;
}

int PrinterToolReadWriteAgent::Send(unsigned char *data, int len) {
    return Send(data, len, 0);
}

int PrinterToolReadWriteAgent::Read(unsigned char *data, int dataSize) {
    return Read(data, dataSize, 0);
}
