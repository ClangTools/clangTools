//
// Created by caesar kekxv on 2020/3/27.
//

#include "ssd1306.h"
#include <logger.h>
#include <opencv_tool.h>

using namespace cv;

ssd1306::ssd1306(int rst, OLED_MODE _mode, const std::string &path) {
    mode = _mode;
    if (_mode == OLED_MODE::I2C) {
#ifdef ENABLE_I2C
        if(!path.empty()){
            i2CTool.setPath(path.c_str());
        }
        for (unsigned char i : i2c_addr) {
            i2CTool.setAddr(i);
            i2CTool.Open();
            if (i2CTool.IsOpen())break;
        }
#endif
    } else {
#ifdef ENABLE_SPI
        if(!path.empty()){
            spiTool.setPath(path.c_str());
            // spiTool.setSpeed(500000);
            spiTool.Open();
        }
#endif
    }
    Initial();
}

int ssd1306::WriteCommand(unsigned char ins) {
#if defined(ENABLE_I2C) or defined (ENABLE_SPI)
#ifdef ENABLE_I2C
    if (!i2CTool.IsOpen()) {
        logger::instance()->e(__FILENAME__, __LINE__, "open fail");
        return -1;
    }
    return i2CTool.Write(std::vector<unsigned char>{0x00, ins});
#endif
#endif
    return -1;
}

int ssd1306::WriteData(unsigned char dat) {
#if defined(ENABLE_I2C) or defined (ENABLE_SPI)
#ifdef ENABLE_I2C
    if (!i2CTool.IsOpen()) {
        logger::instance()->e(__FILENAME__, __LINE__, "open fail");
        return -1;
    }
    return i2CTool.Write(std::vector<unsigned char>{0x40, dat});
#endif
#endif
}

int ssd1306::SetPos(unsigned char x, unsigned char y) {
    int ret = 0;
    ret = WriteCommand(0xb0 + y);
    if (0 > ret)return ret;

    ret = WriteCommand((((x & 0xf0) >> 4) | 0x10));//|0x10
    if (0 > ret)return ret;

    ret = WriteCommand((x & 0x0f) | 0x01);//|0x01
    return ret;

}


int ssd1306::Initial(unsigned char _vccstate) {
    this->vccstate = _vccstate;
    logger::instance()->d(__FILENAME__, __LINE__, "initial");
    int ret = WriteCommand(SSD1306_DISPLAYOFF);//display off
    if (0 > ret)return ret;
    WriteCommand(SSD1306_SETDISPLAYCLOCKDIV);//            # 0xD5
    WriteCommand(0x80);//      # the suggested ratio 0x80
    WriteCommand(SSD1306_SETMULTIPLEX);//    # 0xA8
    WriteCommand(HEIGHT == 64 ? 0x3F : 0x1F);
    WriteCommand(SSD1306_SETDISPLAYOFFSET);//  # 0xD3
    WriteCommand(0x0);//   # no offset
    WriteCommand(SSD1306_SETSTARTLINE | 0x0);//    # line #0
    WriteCommand(SSD1306_CHARGEPUMP);//      # 0x8D
    if (_vccstate == SSD1306_EXTERNALVCC)
        WriteCommand(0x10);
    else
        WriteCommand(0x14);
    WriteCommand(SSD1306_MEMORYMODE);//     # 0x20

    WriteCommand(0x00);//     # 0x0 act like ks0108
    WriteCommand(SSD1306_SEGREMAP | 0x1);//
    WriteCommand(SSD1306_COMSCANDEC);//
    WriteCommand(SSD1306_SETCOMPINS);//       # 0xDA
    WriteCommand(HEIGHT == 64 ? 0x12 : 0x02);//
    WriteCommand(SSD1306_SETCONTRAST);//       # 0x81
    if (HEIGHT == 64) {
        if (_vccstate == SSD1306_EXTERNALVCC)
            WriteCommand(0x9F);
        else
            WriteCommand(0xCF);
    } else
        WriteCommand(0x8F);//

    WriteCommand(SSD1306_SETPRECHARGE);//       # 0xd9
    if (_vccstate == SSD1306_EXTERNALVCC)
        WriteCommand(0x22);
    else
        WriteCommand(0xF1);
    WriteCommand(SSD1306_SETVCOMDETECT);//          # 0xDB
    WriteCommand(0x40);//
    WriteCommand(SSD1306_DISPLAYALLON_RESUME);//      # 0xA4
    WriteCommand(SSD1306_NORMALDISPLAY);//       # 0xA6

    WriteCommand(SSD1306_DISPLAYON);
    logger::instance()->d(__FILENAME__, __LINE__, "initial finish");
    return 0;
}

void ssd1306::DisplayState(bool on) {
    WriteCommand(on ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);    /*display off*/
}

int ssd1306::WriteData(unsigned char x, unsigned char y, unsigned char dat) {
    SetPos(x, y);
    return WriteData(dat);
}

cv::Mat *ssd1306::GetImg() {
    Mat *img = new Mat(HEIGHT, WIDTH, CV_8UC3);
    *img = cv::Scalar(0, 0, 0);
    return img;
}

void ssd1306::free(cv::Mat *&img) {
    delete img;
    img = nullptr;
}

void ssd1306::putText(cv::Mat *img, unsigned int x, unsigned int y, const std::string &text, double fontScale,
                      int thickness) {
    cv::Point p = cv::Point(x, y);
    //加上字符的起始点
    cv::putText(*img, text, p, cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(0, 0, 0), thickness, LINE_AA);
}

int ssd1306::draw(cv::Mat *pMat, bool is_txt) {
    int ret = 0;
    std::vector<std::vector<unsigned char>> outputPtr;
    if (is_txt) {
        Mat dithImg = pMat->clone();
        if (dithImg.type() != CV_8U) {
            cvtColor(dithImg, dithImg, cv::COLOR_BGR2GRAY);
        }
        /* Get the size info */
        int imgWidth = dithImg.cols;
        int imgHeight = dithImg.rows;

        outputPtr = std::vector<std::vector<unsigned char>>(imgHeight, std::vector<unsigned char>(imgWidth));
        for (int i = 0; i < imgHeight; i++) {
            for (int j = 0; j < imgWidth; j++) {
                if (dithImg.at<uint8_t>(i, j) > 127) {
                    outputPtr[i][j] = 255;
                } else {
                    outputPtr[i][j] = 0;
                }
            }
        }
    } else {
        opencv_tool::ImgDithering(*pMat, outputPtr);
    }

    std::vector<std::vector<unsigned char>> points(HEIGHT / 8, std::vector<unsigned char>(WIDTH));
    for (int x = 0; x < WIDTH; x++) {
        for (int i = 0; i < HEIGHT / 8; i++) {
            unsigned char flag = 0;
            for (int _i = 0; _i < 8; _i++) {
                int y = i * 8 + _i;
                flag |= (outputPtr[y][x] == 0 ? 0x01 : 0x00) << _i;
            }
            points[i][x] = flag;
        }
    }
    draw(points);
    return 0;
}

int ssd1306::Fill_Screen(unsigned char dat) {

    int ret = 0;
    ret = WriteCommand(SSD1306_COLUMNADDR);    /*set lower column address*/
    if (0 > ret)return ret;
    WriteCommand(0);//              # Column start address. (0 = reset)
    WriteCommand(WIDTH - 1);//  # Column end address.
    WriteCommand(SSD1306_PAGEADDR);//
    WriteCommand(0);//              # Page start address. (0 = reset)
    WriteCommand(HEIGHT / 8 - 1);// # Page end address.
    for (int y = 0; y < HEIGHT / 8; y++) {
        for (int x = 0; x < WIDTH; x++) {
            WriteData(dat);
        }
    }
    return 0;
}

int ssd1306::draw(std::vector<std::vector<unsigned char>> outputPtr) {
    int ret = 0;
    ret = WriteCommand(SSD1306_COLUMNADDR);    /*set lower column address*/
    if (0 > ret)return ret;
    WriteCommand(0);//              # Column start address. (0 = reset)
    WriteCommand(WIDTH - 1);//  # Column end address.
    WriteCommand(SSD1306_PAGEADDR);//
    WriteCommand(0);//              # Page start address. (0 = reset)
    WriteCommand(HEIGHT / 8 - 1);// # Page end address.
    //# Write buffer data.
    // if self._spi is not None:
    //     ;//# Set DC high for data.
    //     self._gpio.set_high(self._dc)
    //             ;//# Write buffer.
    //     self._spi.write(self._buffer)
    // else:

    for (int y = 0; y < HEIGHT / 8; y++) {
        for (int x = 0; x < WIDTH; x++) {
            WriteData(outputPtr[y][x]);
        }
    }

    return 0;
}

void ssd1306::clear() {
    Fill_Screen(0x00);
}

int ssd1306::GetLineY14(int line) {
    if (line < 1)line = 1;
    if (line > 4)line = 4;
    line--;
    int y = 16 * line + 1;
    return y;
}

ssd1306::~ssd1306() =
default;
