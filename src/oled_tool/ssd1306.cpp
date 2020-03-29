//
// Created by caesar kekxv on 2020/3/27.
//

#include "ssd1306.h"
#include <logger.h>
#include <opencv_tool.h>

using namespace cv;

ssd1306::ssd1306(int rst) {
    // _rst = rst;
    // pinMode(rst, OUTPUT);
    for (unsigned char i : i2c_addr) {
        i2CTool.setAddr(i);
        i2CTool.Open();
        if (i2CTool.IsOpen())break;
    }
    Initial();
}

int ssd1306::WriteCommand(unsigned char ins) {
    if (!i2CTool.IsOpen()) {
        logger::instance()->e(__FILENAME__, __LINE__, "open fail");
        return -1;
    }
    return i2CTool.Write(std::vector<unsigned char>{0x00, ins});
}

int ssd1306::WriteData(unsigned char dat) {
    if (!i2CTool.IsOpen()) {
        logger::instance()->e(__FILENAME__, __LINE__, "open fail");
        return -1;
    }
    return i2CTool.Write(std::vector<unsigned char>{0x40, dat});
}

int ssd1306::IIC_SetPos(unsigned char x, unsigned char y) {
    if (!i2CTool.IsOpen()) {
        logger::instance()->e(__FILENAME__, __LINE__, "open fail");
        return -1;
    }
    int ret = 0;
    ret = WriteCommand(0xb0 + y);
    if (0 > ret)return ret;

    ret = WriteCommand((((x & 0xf0) >> 4) | 0x10));//|0x10
    if (0 > ret)return ret;

    ret = WriteCommand((x & 0x0f) | 0x00);//|0x01
    return ret;
}

int ssd1306::Fill_Screen(unsigned char dat1, unsigned char dat2) {
    int ret = 0;
    unsigned char x, y;

    ret = WriteCommand(0x02);    /*set lower column address*/
    if (0 > ret)return ret;

    ret = WriteCommand(0x10);    /*set higher column address*/
    if (0 > ret)return ret;

    ret = WriteCommand(0xB0);    /*set page address*/
    if (0 > ret)return ret;

    for (y = 0; y < HEIGHT / 8; y++) {
        WriteCommand(0xB0 + y);    /*set page address*/
        WriteCommand(0x02);    /*set lower column address*/
        WriteCommand(0x10);    /*set higher column address*/
        for (x = 0; x < WIDTH / 2; x++) {
            WriteData(dat1);
            WriteData(dat2);
        }
    }
    return 0;
}

int ssd1306::Initial() {
    if (!i2CTool.IsOpen())return -1;
    logger::instance()->d(__FILENAME__, __LINE__, "initial");
    int ret = WriteCommand(0xAE);//display off
    if (0 > ret)return ret;
    // logger::instance()->d(__FILENAME__,__LINE__,"write command 0xAE");

    WriteCommand(0x00);//set lower column address
    WriteCommand(0x10);//set higher column address

    WriteCommand(0x40);//set display start line

    WriteCommand(0xB0);//set page address

    WriteCommand(0x81);//对比度设置
    WriteCommand(0xCF);//0~255（对比度值⋯⋯效果不是特别明显）

    WriteCommand(0xA1);//set segment remap

    WriteCommand(0xA6);//normal / reverse

    WriteCommand(0xA8);//multiplex ratio
    WriteCommand(0x3F);//duty = 1/64

    WriteCommand(0xC8);//Com scan direction

    WriteCommand(0xD3);//set display offset
    WriteCommand(0x00);

    WriteCommand(0xD5);//set osc division
    WriteCommand(0x80);

    WriteCommand(0xD9);//set pre-charge period
    WriteCommand(0xF1);

    WriteCommand(0xDA);//set COM pins
    WriteCommand(0x12);

    WriteCommand(0xDB);//set vcomh
    WriteCommand(0x40);

    WriteCommand(0x8D);//set charge pump enable
    WriteCommand(0x14);

    WriteCommand(0xAF);//display ON
#if 0 //SH1106
    WriteCommand(0xAE);    /*display off*/

    WriteCommand(0x02);    /*set lower column address*/
    WriteCommand(0x10);    /*set higher column address*/

    WriteCommand(0x40);    /*set display start line*/

    WriteCommand(0xB0);    /*set page address*/

    WriteCommand(0x81);    /*contract control*/
    WriteCommand(0x80);    /*128*/

    WriteCommand(0xA1);    /*set segment remap*/

    WriteCommand(0xA6);    /*normal / reverse*/

    WriteCommand(0xA8);    /*multiplex ratio*/
    WriteCommand(0x3F);    /*duty = 1/32*/

    WriteCommand(0xad);    /*set charge pump enable*/
    WriteCommand(0x8b);     /*    0x8a    外供VCC   */

    WriteCommand(0x30);    /*0X30---0X33  set VPP   9V 电压可以调整亮度!!!!*/

    WriteCommand(0xC8);    /*Com scan direction*/

    WriteCommand(0xD3);    /*set display offset*/
    WriteCommand(0x00);   /*   0x20  */

    WriteCommand(0xD5);    /*set osc division*/
    WriteCommand(0x80);

    WriteCommand(0xD9);    /*set pre-charge period*/
    WriteCommand(0x1f);    /*0x22*/

    WriteCommand(0xDA);    /*set COM pins*/
    WriteCommand(0x12);//0x02 -- duanhang xianshi,0x12 -- lianxuhang xianshi!!!!!!!!!

    WriteCommand(0xdb);    /*set vcomh*/
    WriteCommand(0x40);


    WriteCommand(0xAF);    /*display ON*/
#endif
    return 0;
}

void ssd1306::DisplayState(bool on) {
    WriteCommand(on ? 0xAF : 0xAE);    /*display off*/
}

int ssd1306::WriteData(unsigned char x, unsigned char y, unsigned char dat) {
    IIC_SetPos(x, y);
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
    cv::putText(*img, text, p, cv::FONT_HERSHEY_TRIPLEX, fontScale, cv::Scalar(255, 255, 255), thickness);
    //在图像上加字符
    //第一个参数为要加字符的目标函数
    //第二个参数为要加的字符
    //第三个参数为字体
    //第四个参数为子的粗细
    //第五个参数为字符的颜色

}

int ssd1306::draw(cv::Mat *pMat) {
    int ret = 0;
    std::vector<std::vector<unsigned char>> outputPtr;
    // setenv("DISPLAY", "localhost:10.0", 1);
    // imshow("p", *pMat);
    // waitKey();
    opencv_tool::ImgDithering(*pMat, outputPtr);

    std::vector<std::vector<unsigned char>> points(HEIGHT / 8, std::vector<unsigned char>(WIDTH));
    for (int x = 0; x < WIDTH; x++) {
        for (int i = 0; i < HEIGHT / 8; i++) {
            unsigned char flag = 0;
            for (int _i = 0; _i < 8; _i++) {
                int y = i * 8 + _i;
                flag |= (outputPtr[y][x] == 0 ? 0x00 : 0x01) << _i;
            }
            points[i][x] = flag;
        }
    }
    draw(points);
    return 0;
}

int ssd1306::Fill_Screen(unsigned char dat) {
    int ret = 0;
    unsigned char x, y;

    ret = WriteCommand(0x02);    /*set lower column address*/
    if (0 > ret)return ret;

    ret = WriteCommand(0x10);    /*set higher column address*/
    if (0 > ret)return ret;

    ret = WriteCommand(0xB0);    /*set page address*/
    if (0 > ret)return ret;

    for (y = 0; y < HEIGHT / 8; y++) {
        WriteCommand(0xB0 + y);    /*set page address*/
        WriteCommand(0x02);    /*set lower column address*/
        WriteCommand(0x10);    /*set higher column address*/
        for (x = 0; x < WIDTH; x++) {
            WriteData(dat);
        }
    }
    return 0;
}

int ssd1306::draw(std::vector<std::vector<unsigned char>> outputPtr) {
    int ret = 0;
    unsigned char x, y;

    ret = WriteCommand(0x02);    /*set lower column address*/
    if (0 > ret)return ret;

    ret = WriteCommand(0x10);    /*set higher column address*/
    if (0 > ret)return ret;

    ret = WriteCommand(0xB0);    /*set page address*/
    if (0 > ret)return ret;

    for (y = 0; y < HEIGHT / 8; y++) {
        WriteCommand(0xB0 + y);    /*set page address*/
        WriteCommand(0x02);    /*set lower column address*/
        WriteCommand(0x10);    /*set higher column address*/
        for (x = 0; x < WIDTH; x++) {
            WriteData(outputPtr[y][x]);
        }
    }
    return 0;
}

ssd1306::~ssd1306() =
default;
