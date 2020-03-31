//
// Created by caesar kekxv on 2020/3/27.
//

#include <ssd1306.h>
#include <logger.h>
#include <opencv2/opencv.hpp>
#include <CvxFont.h>
#include <opencv_tool.h>
#include <net_tool.h>


int main(int argc, char *argv[]) {
    if (!getenv("DISPLAY"))setenv("DISPLAY", "localhost:10.0", 1);
    logger::instance()->init_default();
    logger::instance()->d(__FILENAME__, __LINE__, "start");
    ssd1306 ssd1306;
    ssd1306.clear();

#ifdef ENABLE_FREETYPE
    cv::Mat img(64, 128, CV_8UC3, cv::Scalar(255, 255, 255)); // create a black background

    cvx::CvxFont font(logger::get_local_path() + logger::path_split + "OpenDotMatrixFont.ttf");
    cvx::CvxFont font1(logger::get_local_path() + logger::path_split + "seguisym.ttf");


    cv::String msg7 = "柒凯计算机软件服务";
    font.putText(img, msg7, cv::Point(0, ssd1306::GetLineY14(4)));

    font.putText(img, "blog.kekxv.com", cv::Point(0, ssd1306::GetLineY14(0) * 2 / 3), 12);
    std::vector<std::string> ips;
    net_tool::GetIP(ips);
    int line = 2;
    for (auto & ip : ips) {
        if (ip.find("127") != std::string::npos)continue;
        font.putText(img, ip, cv::Point(0, ssd1306::GetLineY14(line++) * 2 / 3), 12);
    }

    cv::Mat qrImg = opencv_tool::CreateQrCode("https://blog.kekxv.com/", 1);
    cv::Mat roi = img(
            cv::Rect(127 - qrImg.size().width, ssd1306::GetLineY14(4) - qrImg.size().height, qrImg.cols,
                     qrImg.rows));
    qrImg.copyTo(roi);

    std::vector<uint32_t> symbols;
    symbols.push_back((uint32_t) 0x1f300 + 32 * (14 + 9 - 1) + 2 - 1);
    font1.putText(img, symbols, cv::Point(127 - qrImg.size().width - 14, ssd1306::GetLineY14(3)));


    ssd1306.draw(&img, true);

#endif

    logger::instance()->d(__FILENAME__, __LINE__, "end");
    // ssd1306.DisplayState(false);
    return 0;
}

