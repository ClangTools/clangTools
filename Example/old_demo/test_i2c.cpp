//
// Created by caesar kekxv on 2020/3/4.
//
// 树莓派电池模块
//     读取电池电压以及电池电量 🔋
//

#include <i2c_tool.h>
#include <logger.h>

int file_i2c;
int length;
unsigned char buffer[60] = {0};

int main(int argc, char *argv[]) {
    logger::instance()->init_default();

    i2c_tool i2CTool;
    int ret;
    double p, v;
    unsigned char dataV, dataP;

    i2CTool.setAddr(0x36);
    i2CTool.Open();

    /**
     * use ioctl transfer data
     */
    std::vector<unsigned char> data;
    std::vector<unsigned char> wData;
    wData.push_back(0x02);
    ret = i2CTool.transfer(wData, &data, 1);
    if (ret <= 0) {
        logger::instance()->e(TAG, __LINE__, "Read 0x02 : %d", ret);
        return -1;
    }
    dataV = data[0];
    v = (double) (((dataV & 0xFF) << 8) + (dataV >> 8)) * 1.25 / 1000 / 16;
    data.clear();
    wData[0] = 0x04;
    ret = i2CTool.transfer(wData, &data, 1);
    if (ret <= 0) {
        logger::instance()->e(TAG, __LINE__, "Read 0x04 : %d", ret);
        return -2;
    }
    dataP = data[0];
    p = (double) ((int) ((dataP & 0xFF) << 8) + (int) (dataP >> 8)) / 256 - 5;

    logger::instance()->i(TAG, __LINE__, "电压 : %.02lfV; 电量 : %.02lf %%", v, p);
    /**
     * use ioctl transfer data end
     */

    unsigned char d[] = {0x02, 0x00};
    ret = i2CTool.Write(d, 1);
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Write 0x02 : %d", ret);
        return -1;
    }
    data.clear();
    ret = i2CTool.Read(data, 1);
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Read 0x02 : %d", ret);
        return -1;
    }
    dataV = data[0];
    v = (double) (((dataV & 0xFF) << 8) + (dataV >> 8)) * 1.25 / 1000 / 16;
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Read 0x04 : %d", ret);
        return -2;
    }
    d[0] = 0x04;
    ret = i2CTool.Write(d, 1);
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Write 0x02 : %d", ret);
        return -1;
    }
    data.clear();
    ret = i2CTool.Read(data, 1);
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Read 0x02 : %d", ret);
        return -1;
    }
    dataP = data[0];
    p = (double) ((int) ((dataP & 0xFF) << 8) + (int) (dataP >> 8)) / 256 - 5;
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Read 0x04 : %d", ret);
        return -2;
    }
    logger::instance()->i(TAG, __LINE__, "电压 : %.02lfV; 电量 : %.02lf %%", v, p);

    return 0;
}