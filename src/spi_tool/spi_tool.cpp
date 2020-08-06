//
// Created by caesar kekxv on 2020/3/17.
//

#include "spi_tool.h"
#include <cstdint>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <logger.h>

void spi_tool::setPath(const char *path) {
    if (!path)return;
    spi_path = path;
}

bool spi_tool::Open() {
    handle = open(spi_path.c_str(), O_RDWR);
    if (handle < 0) {
        logger::instance()->e(TAG, __LINE__, "can't open device : %s", strerror(errno));
        return false;
    }
    /*
     * spi mode
     */
    int ret = ioctl(handle, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
        logger::instance()->e(TAG, __LINE__, "can't set spi mode : %s", strerror(errno));
        Close();
        return false;
    }
    ret = ioctl(handle, SPI_IOC_RD_MODE, &mode);
    if (ret == -1) {
        logger::instance()->e(TAG, __LINE__, "can't get spi mode : %s", strerror(errno));
        Close();
        return false;
    }
    /*
     * bits per word
     */
    ret = ioctl(handle, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
        logger::instance()->e(TAG, __LINE__, "can't set bits per word : %s", strerror(errno));
        Close();
        return false;
    }

    ret = ioctl(handle, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {
        logger::instance()->e(TAG, __LINE__, "can't get bits per word : %s", strerror(errno));
        Close();
        return false;
    }

    /*
     * max speed hz
     */
    ret = ioctl(handle, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        logger::instance()->e(TAG, __LINE__, "can't set max speed hz : %s", strerror(errno));
        Close();
        return false;
    }
    ret = ioctl(handle, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        logger::instance()->e(TAG, __LINE__, "can't get max speed hz : %s", strerror(errno));
        Close();
        return false;
    }
    return true;
}

void spi_tool::Close() {
    close(handle);
    handle = -1;
}

bool spi_tool::isOpen() {
    return handle > 0;
}

int spi_tool::transfer(std::vector<unsigned char> sData, std::vector<unsigned char> &rData, uint16_t delay) {
    auto *tx = new uint8_t[sData.size()];
    memcpy(tx, sData.data(), sData.size());
    auto *rx = new uint8_t[sData.size()];
    memset(rx, 0x00, sData.size());

    struct spi_ioc_transfer tr = {};
    tr.tx_buf = (unsigned long) tx;
    tr.rx_buf = (unsigned long) rx;
    tr.len = sData.size();
    tr.delay_usecs = delay;
    tr.speed_hz = speed;
    tr.bits_per_word = bits;

    int ret = ioctl(handle, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        logger::instance()->e(TAG, __LINE__, "can't send spi message : %s", strerror(errno));
        delete[]rx;
        delete[]tx;
        return -99;
    }
    rData.insert(rData.end(), &rx[0], &rx[ret]);
    delete[]rx;
    delete[]tx;
    return ret;
}

void spi_tool::setSpeed(uint16_t v) {
    this->speed = v;
}

void spi_tool::setBits(uint16_t v) {
    bits = v;
}

void spi_tool::setMode(uint16_t _mode) {
/**
                {"loop",    0, 0, 'l'},
                {"cpha",    0, 0, 'H'},
                {"cpol",    0, 0, 'O'},
                {"lsb",     0, 0, 'L'},
                {"cs-high", 0, 0, 'C'},
                {"3wire",   0, 0, '3'},
                {"no-cs",   0, 0, 'N'},
                {"ready",   0, 0, 'R'},
 */
    /*
               switch (c) {
                   case 'l':
                       _mode |= SPI_LOOP;
                       break;
                   case 'H':
                       _mode |= SPI_CPHA;
                       break;
                   case 'O':
                       _mode |= SPI_CPOL;
                       break;
                   case 'L':
                       _mode |= SPI_LSB_FIRST;
                       break;
                   case 'C':
                       _mode |= SPI_CS_HIGH;
                       break;
                   case '3':
                       _mode |= SPI_3WIRE;
                       break;
                   case 'N':
                       _mode |= SPI_NO_CS;
                       break;
                   case 'R':
                       _mode |= SPI_READY;
                       break;
               }
               */
}


