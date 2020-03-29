//
// Created by caesar kekxv on 2020/3/27.
//

#ifndef TOOLS_SSD1306_H
#define TOOLS_SSD1306_H


#include <cstdint>
#include <src/i2c_tool/i2c_tool.h>

namespace cv {
    class Mat;
}
class ssd1306 {
public:
    typedef enum {
        SPI,
        I2C
    } OLED_MODE;
    typedef enum {
        OLED_CMD,
        OLED_DATA
    } OLED_WR_MODE;

    typedef enum {
        Display_ON,
        Display_OFF,
        Display_Clear,
        Display_Test
    } DIS_MODE;

    explicit ssd1306(int rst = 0);

    ~ssd1306();


    int WriteCommand(unsigned char ins);

    int WriteData(unsigned char dat);

    int WriteData(unsigned char x, unsigned char y, unsigned char dat);

    int IIC_SetPos(unsigned char x, unsigned char y);

    int Fill_Screen(unsigned char dat1, unsigned char dat2);
    int Fill_Screen(unsigned char dat);

    cv::Mat *GetImg();

    static void free(cv::Mat *&img);

    static void putText(cv::Mat *img, unsigned int x, unsigned int y, const std::string &text, double fontScale = 1,
                        int thickness = 1);

    int Initial();

    void DisplayState(bool flag);

    int draw(cv::Mat *pMat);
    int draw(std::vector<std::vector<unsigned char>> outputPtr);

private:
    int _rst = 0;
    const unsigned char i2c_addr[2]{0x3c, 0x3d};
    i2c_tool i2CTool;

    unsigned char WIDTH = 128;
    unsigned char HEIGHT = 64;
};


#endif //TOOLS_SSD1306_H
