//
// Created by caesar kekxv on 2020/3/27.
//

#include <ssd1306.h>
#include <logger.h>

void ShowCN(ssd1306 &ssd1306, unsigned char x, unsigned char y, unsigned char N);

int main(int argc, char *argv[]) {
    logger::instance()->init_default();
    logger::instance()->d(__FILENAME__, __LINE__, "start");
    ssd1306 ssd1306;
    ssd1306.Fill_Screen(0x00);
    // ssd1306.Fill_Screen(0xFF, 0xFF);
    // ssd1306.Fill_Screen(0xFF / 2,0b11100111);

    // ssd1306.WriteData(000, 0, 0b11111111);
    // ssd1306.WriteData(003, 0, 0b11100111);
    // ssd1306.WriteData(064, 0, 0b11111111);
    // ssd1306.WriteData(067, 0, 0b11100111);
    // ssd1306.WriteData(120, 0, 0b11111111);
    // ssd1306.WriteData(127, 0, 0b11100111);

    auto *img = ssd1306.GetImg();
    ssd1306::putText(img,0,10,"1234567890",0.3,1);

    ssd1306.draw(img);

    ssd1306::free(img);
    logger::instance()->d(__FILENAME__, __LINE__, "end");
    // ssd1306.DisplayState(false);
    return 0;
}

