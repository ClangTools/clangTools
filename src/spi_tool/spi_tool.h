//
// Created by caesar kekxv on 2020/3/17.
//

#ifndef TOOLS_SPI_TOOL_H
#define TOOLS_SPI_TOOL_H


class spi_tool {

public:
    void setPath(const char *path);
    void setDelay(uint16_t Delay);
    void setSpeed(uint16_t speed);
    void setBits(uint16_t bits);
    void setMode(uint16_t mode);

    bool Open();

private:

    std::string spi_path = "/dev/spidev0.0";
    int handle = -1;
    uint8_t mode;
    uint8_t bits = 8;
    uint32_t speed = 500000;
    uint16_t delay;
};


#endif //TOOLS_SPI_TOOL_H
