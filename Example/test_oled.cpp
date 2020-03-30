//
// Created by caesar kekxv on 2020/3/27.
//

#include <ssd1306.h>
#include <logger.h>
#include <opencv2/opencv.hpp>
#include <CvxFont.h>
#include <opencv_tool.h>


void ShowCN(ssd1306 &ssd1306, unsigned char x, unsigned char y, unsigned char N);

int main(int argc, char *argv[]) {
    if (!getenv("DISPLAY"))setenv("DISPLAY", "localhost:10.0", 1);
    logger::instance()->init_default();
    logger::instance()->d(__FILENAME__, __LINE__, "start");
    ssd1306 ssd1306;
    ssd1306.clear();

#if (0)
    {
        ssd1306.WriteData(000, 0, 0b11111111);
        ssd1306.WriteData(003, 0, 0b11100111);
        ssd1306.WriteData(064, 0, 0b11111111);
        ssd1306.WriteData(067, 0, 0b11100111);
        ssd1306.WriteData(120, 0, 0b11111111);
        ssd1306.WriteData(127, 0, 0b11100111);
    }
#endif

#if (0)
    {
        auto *img = ssd1306.GetImg();
        ssd1306::putText(img, 3, 12, "1234567890^&*()", 0.4, 1);
        ssd1306::putText(img, 3, 22, "ABCDEFGHIJKLMN#", 0.4, 1);
        ssd1306::putText(img, 3, 32, "OPQRSTUVWXYZ?,", 0.4, 1);
        ssd1306::putText(img, 3, 42, "abcdefghijklmn@\\", 0.4, 1);
        ssd1306::putText(img, 3, 52, "opqrstuvwxyz<>!", 0.4, 1);
        ssd1306::putText(img, 3, 62, "+-=_{}[]:;\"'./$%", 0.4, 1);

        ssd1306.draw(img, true);

        ssd1306::free(img);
    }
#endif

#if (0)
    for(int i=0;i<1000;i++){
        auto *img = ssd1306.GetImg();
        ssd1306::putText(img, 5, 32, "time:" + std::to_string(logger::get_time_tick()));
        ssd1306.draw(img, true);
        ssd1306::free(img);
    }
#endif
#if (0)
    // cv::Mat img = cv::imread("/home/pi/test.bmp");
    cv::Mat img = cv::imread("/home/pi/柒凯计算机软件服务.jpg");
    if (img.empty())return 1;
    ssd1306.draw(&img, true);
#endif
#ifdef ENABLE_FREETYPE
    cv::Mat img(64, 128, CV_8UC3, cv::Scalar(255, 255, 255)); // create a black background

    cvx::CvxFont font(logger::get_local_path() + logger::path_split + "OpenDotMatrixFont.ttf");
    cvx::CvxFont font1(logger::get_local_path() + logger::path_split + "seguisym.ttf");


    cv::String msg7 = "柒凯计算机软件服务";
    font.putText(img, msg7, cv::Point(0, ssd1306::GetLineY14(4)));

    std::vector<uint32_t> symbols;
    symbols.push_back((uint32_t) 0x1f300 + 32 * (14 + 9 - 1) + 2 - 1);
    font1.putText(img, symbols, cv::Point((9 - 1) * 14, ssd1306::GetLineY14(3)));


    cv::Mat qrImg = opencv_tool::CreateQrCode("https://blog.kekxv.com/");

    cv::Mat roi = img(cv::Rect(0, 0, qrImg.cols, qrImg.rows));
    qrImg.copyTo(roi);

    ssd1306.draw(&img, true);

#endif

    logger::instance()->d(__FILENAME__, __LINE__, "end");
    // ssd1306.DisplayState(false);
    return 0;
}

