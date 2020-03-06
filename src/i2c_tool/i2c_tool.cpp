//
// Created by caesar kekxv on 2020/3/5.
//

#include "i2c_tool.h"
#include <unistd.h>                //Needed for I2C port
#include <fcntl.h>                //Needed for I2C port
#include <sys/ioctl.h>            //Needed for I2C port
#include <iostream>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>        //Needed for I2C port
#include <logger.h>

using namespace std;

i2c_tool::i2c_tool() = default;

i2c_tool::~i2c_tool() {
    Close();
}

void i2c_tool::setPath(const char *path) {
    if (path == nullptr) {
        i2c_path = "";
        return;
    }
    i2c_path = path;
}

unsigned char i2c_tool::setAddr(unsigned char addr) {
    Addr = addr;
    return Addr;
}

int i2c_tool::Open() {
    if ((i2c_handle = open(i2c_path.c_str(), O_RDWR)) < 0) {
        //ERROR HANDLING: you can check errno to see what went wrong
        logger::instance()->e(__FILENAME__, __LINE__,
                              "Failed to open the i2c bus");
        return Type::FailOpen;
    }
    int addr = Addr;//<<<<<The I2C address of the slave
    if (ioctl(i2c_handle, I2C_SLAVE, addr) < 0) {
        //ERROR HANDLING; you can check errno to see what went wrong
        logger::instance()->e(__FILENAME__, __LINE__,
                              "Failed to acquire bus access and/or talk to slave.");
        Close();
        return FailAddr;
    }
    ioctl(i2c_handle, I2C_TIMEOUT, 100);
    ioctl(i2c_handle, I2C_RETRIES, 2);
    return 0;
}

bool i2c_tool::IsOpen() {
    return i2c_handle > 0;
}

int i2c_tool::Close() {
    if (IsOpen())close(i2c_handle);
    i2c_handle = -1;
    return 0;
}

int i2c_tool::Read(std::vector<unsigned char> &data, int read_len, int timeout) {
    if (!IsOpen())return Type::FailNotOpen;
    auto *buffer = new unsigned char[read_len+1];
    memset(buffer, 0x00, read_len+1);
    //----- READ BYTES -----
    int length = read_len;            //<<< Number of bytes to read
    int ret = read(i2c_handle, buffer, length);
    // read() returns the number of bytes actually read,
    // if it doesn't match then an error occurred
    // (e.g. no response from the device)
    if (ret != length) {
        //ERROR HANDLING: i2c transaction failed
        logger::instance()->e(__FILENAME__, __LINE__,
                              "failed to read from the i2c bus.return code : %d [ %s ]", ret,strerror(errno));
        delete[]buffer;
        return Type::FailRead;
    }
    data.insert(data.end(), &buffer[0], &buffer[ret]);
    delete[]buffer;
    return ret;
}

int i2c_tool::Write(std::vector<unsigned char> data, int timeout) {
    return Write(data.data(), data.size(), timeout);
}

int i2c_tool::Write(unsigned char *data, int data_len, int timeout) {
    if (!IsOpen())return Type::FailNotOpen;
    int ret = write(i2c_handle, data, data_len);
    // write() returns the number of bytes actually written,
    // if it doesn't match then an error occurred
    // (e.g. no response from the device)
    if (ret != data_len) {
        /* ERROR HANDLING: i2c transaction failed */
        logger::instance()->e(__FILENAME__, __LINE__,
                              "failed to write to the i2c bus.return code : %d", ret);
    }
    return ret;
}

int i2c_tool::transfer(std::vector<unsigned char> wData, std::vector<unsigned char> *rData, int read_len,
                       int timeout) {
    if (!IsOpen())return Type::FailNotOpen;
    struct i2c_rdwr_ioctl_data i2c_data{};
    auto *buff = new unsigned char[read_len + 1];
    memset(buff,0x00,read_len + 1);
    int ret = 0;
    i2c_data.nmsgs = (rData == nullptr || read_len <= 0) ? 1 : 2;
    i2c_data.msgs = (struct i2c_msg *) malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));
    if (i2c_data.msgs == nullptr) {
        logger::instance()->e(__FILENAME__, __LINE__,
                              "mallocerror:%s", strerror(errno));
        delete[]buff;
        free(i2c_data.msgs);
        return FailRead;
    }
    ioctl(i2c_handle, I2C_TIMEOUT, timeout);
    unsigned char device_addr = Addr;// >> 1;

    //write reg
    i2c_data.msgs[0].len = wData.size();
    i2c_data.msgs[0].addr = device_addr;
    i2c_data.msgs[0].flags = 0;     // 0: write 1:read
    i2c_data.msgs[0].buf = wData.data();
    if (read_len > 0 && rData != nullptr) {
        //read data
        i2c_data.msgs[1].len = read_len;
        i2c_data.msgs[1].addr = device_addr;
        i2c_data.msgs[1].flags = 1;     // 0: write 1:read
        i2c_data.msgs[1].buf = buff;
    }

    ret = ioctl(i2c_handle, I2C_RDWR, (unsigned long) &i2c_data);
    if (ret < 0) {
        logger::instance()->e(__FILENAME__, __LINE__,
                              "0x%02X transfer data error,ret:%d [ %s ]", device_addr, ret,strerror(errno));
        free(i2c_data.msgs);
        delete[]buff;
        return FailRead;
    }
    if (read_len > 0 && rData != nullptr) {
        rData->insert(rData->end(), &buff[0], &buff[read_len]);
        free(i2c_data.msgs);
        delete[]buff;
        return read_len;
    }
    free(i2c_data.msgs);
    delete[]buff;
    return ret;
}