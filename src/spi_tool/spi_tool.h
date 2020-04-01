//
// Created by caesar kekxv on 2020/3/17.
//

#ifndef TOOLS_SPI_TOOL_H
#define TOOLS_SPI_TOOL_H

#include <vector>
#include <string>
#include <cstdint>

class spi_tool {

public:
    void setPath(const char *path);

    void setSpeed(uint16_t speed);

    void setBits(uint16_t bits);

    void setMode(uint16_t _mode);

    bool Open();

    void Close();

    bool isOpen();

    int transfer(std::vector<unsigned char> sData, std::vector<unsigned char> &rData, uint16_t delay = 1000);


private:

    std::string spi_path = "/dev/spidev0.0";
    int handle = -1;
    uint8_t mode;
    uint8_t bits = 8;
    uint32_t speed = 500000;


};


#endif //TOOLS_SPI_TOOL_H
