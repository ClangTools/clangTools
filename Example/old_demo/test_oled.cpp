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
    logger::instance()->d(TAG, __LINE__, "start");
    ssd1306 ssd1306;
    ssd1306.clear();

#ifdef ENABLE_FREETYPE
    cv::Mat srcImg(64, 128, CV_8UC3, cv::Scalar(255, 255, 255)); // create a black background
    cvx::CvxFont font(logger::get_local_path() + logger::path_split + "OpenDotMatrixFont.ttf");
    cvx::CvxFont font1(logger::get_local_path() + logger::path_split + "seguisym.ttf");
    {
        // 初始化
        std::vector<uint32_t> symbols;
        // symbols.push_back((uint32_t) 0x1f300 + 32 * (14 + 9 - 1) + 2 - 1); // 📂
        symbols.push_back((uint32_t) 0x1f300 + 32 * (2 - 1) + 1 - 1);   // 🌠
        font1.putText(srcImg, symbols, cv::Point(127 - 14, ssd1306::GetLineY14(2)), 14);


        cv::String msg7 = "柒凯计算机服务";
        font.putText(srcImg, msg7, cv::Point(0, ssd1306::GetLineY14(4)));

        cv::Mat qrImg = opencv_tool::CreateQrCode("https://blog.kekxv.com/", 1);
        cv::Mat roi = srcImg(
                cv::Rect(127 - qrImg.size().width, ssd1306::GetLineY14(4) - qrImg.size().height / 2, qrImg.cols,
                         qrImg.rows));
        qrImg.copyTo(roi);
    }


    std::vector<std::string> ips;
    for (int i = 10000; i > 0; i--) {
        cv::Mat img = srcImg.clone();
        font.putText(img, logger::GetTime("%Y/%m/%d"), cv::Point(0, ssd1306::GetLineY14(0) * 2 / 3), 10);
        font.putText(img, logger::GetTime("%H:%M:%S"), cv::Point(127 - 8 * 10 * 3 / 5, ssd1306::GetLineY14(0) * 2 / 3),
                     10);
        if (i % 10 == 0) {
            ips.clear();
            net_tool::GetIP(ips);
        }
        int line = 2;
        for (auto &ip : ips) {
            if (ip.find("127") != std::string::npos)continue;
            font.putText(img, ip, cv::Point(0, ssd1306::GetLineY14(line++) * 2 / 3), 12);
        }
        ssd1306.draw(&img, true);
        usleep(50 * 1000);
        if (i < 1)i = 10000;
    }


#endif

    logger::instance()->d(TAG, __LINE__, "end");
    // ssd1306.DisplayState(false);
    return 0;
}

