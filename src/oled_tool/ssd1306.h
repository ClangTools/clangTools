//
// Created by caesar kekxv on 2020/3/27.
//

#ifndef TOOLS_SSD1306_H
#define TOOLS_SSD1306_H


#include <cstdint>
#include <string>
#include <vector>

#ifdef ENABLE_I2C
#include <i2c_tool.h>
#endif
#ifdef ENABLE_SPI
#include <spi_tool.h>
#endif

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

    typedef enum {
        // # Constants
        SSD1306_I2C_ADDRESS = 0x3C,    //# 011110+SA0+RW - 0x3C or 0x3D
        SSD1306_SETCONTRAST = 0x81,
        SSD1306_DISPLAYALLON_RESUME = 0xA4,
        SSD1306_DISPLAYALLON = 0xA5,
        SSD1306_NORMALDISPLAY = 0xA6,
        SSD1306_INVERTDISPLAY = 0xA7,
        SSD1306_DISPLAYOFF = 0xAE,
        SSD1306_DISPLAYON = 0xAF,
        SSD1306_SETDISPLAYOFFSET = 0xD3,
        SSD1306_SETCOMPINS = 0xDA,
        SSD1306_SETVCOMDETECT = 0xDB,
        SSD1306_SETDISPLAYCLOCKDIV = 0xD5,
        SSD1306_SETPRECHARGE = 0xD9,
        SSD1306_SETMULTIPLEX = 0xA8,
        SSD1306_SETLOWCOLUMN = 0x00,
        SSD1306_SETHIGHCOLUMN = 0x10,
        SSD1306_SETSTARTLINE = 0x40,
        SSD1306_MEMORYMODE = 0x20,
        SSD1306_COLUMNADDR = 0x21,
        SSD1306_PAGEADDR = 0x22,
        SSD1306_COMSCANINC = 0xC0,
        SSD1306_COMSCANDEC = 0xC8,
        SSD1306_SEGREMAP = 0xA0,
        SSD1306_CHARGEPUMP = 0x8D,
        SSD1306_EXTERNALVCC = 0x1,
        SSD1306_SWITCHCAPVCC = 0x2,

        // # Scrolling constants
        SSD1306_ACTIVATE_SCROLL = 0x2F,
        SSD1306_DEACTIVATE_SCROLL = 0x2E,
        SSD1306_SET_VERTICAL_SCROLL_AREA = 0xA3,
        SSD1306_RIGHT_HORIZONTAL_SCROLL = 0x26,
        SSD1306_LEFT_HORIZONTAL_SCROLL = 0x27,
        SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL = 0x29,
        SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL = 0x2A,
    } SSD1306_CONFIG;


    explicit ssd1306(int rst = 0, OLED_MODE mode = OLED_MODE::I2C, const std::string &path = "");

    ~ssd1306();


    int WriteData(unsigned char x, unsigned char y, unsigned char dat);

    int SetPos(unsigned char x, unsigned char y);

    int Fill_Screen(unsigned char dat);

    void clear();

    cv::Mat *GetImg();

    static void free(cv::Mat *&img);

    static void putText(cv::Mat *img, unsigned int x, unsigned int y, const std::string &text, double fontScale = 0.35,
                        int thickness = 1);

    int Initial(unsigned char _vccstate = SSD1306_SWITCHCAPVCC);

    void DisplayState(bool flag);

    int draw(cv::Mat *pMat, bool is_txt = false);

    int draw(std::vector<std::vector<unsigned char>> outputPtr);

    static int GetLineY14(int line);
    static int GetLineY16(int line);

private:
    int WriteCommand(unsigned char ins);

    int WriteData(unsigned char dat);

    int _rst = 0;
    const unsigned char i2c_addr[2]{0x3c, 0x3d};
    OLED_MODE mode = I2C;
#ifdef ENABLE_I2C
    i2c_tool i2CTool;
#endif
#ifdef ENABLE_SPI
    spi_tool spiTool;
#endif

    unsigned char vccstate = SSD1306_SWITCHCAPVCC;

    unsigned char WIDTH = 128;
    unsigned char HEIGHT = 64;
};


#endif //TOOLS_SSD1306_H
