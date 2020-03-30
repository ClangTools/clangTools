//
// Created by caesar kekxv on 2020/3/30.
//

#include <src/logger/logger.h>
#include "CvxFont.h"

cv::Scalar randomColor(cv::RNG &rng);

cv::Scalar randomColor(cv::RNG &rng) {
    auto icolor = (unsigned) rng;
    return cv::Scalar(icolor & 255, (icolor >> 8) & 255, (icolor >> 16) & 255);
}

static cv::RNG rng(0xFFFFFFFF);

void testLowerAlpha(cvx::CvxFont &font, cv::Mat &img) {
    cv::String msg = "abcdefghijklmnopqrstuvwxyz";
    int fontSize = 32;
    cvx::putText(img, msg, cv::Point(40, 20), font, fontSize, cv::Scalar(0, 255, 0));
}

void testUpperAlpha(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    cv::String msg1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int fontSize = 32;
    cvx::putText(img, msg1, cv::Point(40, 100), font, fontSize, randomColor(rng));
}

void testHorizon(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    cv::String msg2 = "这是水平字符";
    cvx::putText(img, msg2, cv::Point(40, 200), font, fontSize, randomColor(rng));
}

void testVertical(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    font.setVertical(true);
    cv::String msg3 = "这是竖直字符ada";
    cvx::putText(img, msg3, cv::Point(0, 40), font, fontSize, randomColor(rng));
}

void testHorizonRotate(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    float angle = 30;
    font.setRotateAngle(angle);
    cv::String msg4 = "This is a rotate string!";
    cvx::putText(img, msg4, cv::Point(40, 250), font, fontSize, randomColor(rng));
}

void testVerticalRotate(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    float angle = -30;
    font.setVertical(true);
    font.setRotateAngle(angle);
    cv::String msg5 = "这是旋转字符";
    cvx::putText(img, msg5, cv::Point(700, 100), font, fontSize, randomColor(rng));
}

void testTruncate(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    cv::String msg6 = "字符可以被截断";
    putText(img, msg6, cv::Point(600, 40), font, fontSize, randomColor(rng));
}

void testFontSize(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 16;
    cv::String msg7 = "这是16号字体";
    putText(img, msg7, cv::Point(40, 350), font, fontSize, randomColor(rng));
    cv::String msg8 = "这是32号字体";
    fontSize = 32;
    putText(img, msg8, cv::Point(160, 350), font, fontSize, randomColor(rng));
}

void testSpace(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    cv::String msg9 = "有空 格时";
    putText(img, msg9, cv::Point(500, 300), font, fontSize, randomColor(rng));
}

void testBetweenSpace(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    cv::String msg10 = "更大的字间距";
    font.setFontRatio(0.3f);
    putText(img, msg10, cv::Point(500, 350), font, fontSize, randomColor(rng));
}

void testTransparency(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    cv::String msg11 = "设置透明度";
    font.setDiaphaneity(0.3f);
    putText(img, msg11, cv::Point(500, 200), font, fontSize, randomColor(rng));
}

void testUnderline(cvx::CvxFont &font, cv::Mat &img) {
    //    cv::RNG rng(0xFFFFFFFF);
    int fontSize = 32;
    cv::String msg12 = "加下划线";
    font.setUnderline(true);
    putText(img, msg12, cv::Point(500, 150), font, fontSize, randomColor(rng));
}

void testChinese() {
    cv::Mat img(400, 800, CV_8UC3, cv::Scalar(0, 0, 0)); // create a black background

    cvx::CvxFont font(logger::get_local_path() + logger::path_split + "OpenDotMatrixFont.ttf");
    if(!font.isLoad())return ;

    testLowerAlpha(font, img);
    testUpperAlpha(font, img);
    testHorizon(font, img);
    testVertical(font, img);
    testHorizonRotate(font, img);
    testVerticalRotate(font, img);
    testTruncate(font, img);
    testFontSize(font, img);
    testSpace(font, img);
    testBetweenSpace(font, img);
    testTransparency(font, img);
    testUnderline(font, img);

    int fontSize = 9;
    cv::String msg7 = "柒凯计算机软件服务";
    putText(img, msg7, cv::Point(20, 50), font, fontSize, randomColor(rng));

    cv::imshow("test", img);
    cv::waitKey();
}
// http://www.kreativekorp.com/charset/font/SegoeUISymbol/
void testSymbols() {
    // create a black background
    cv::Mat img(700, 1200, CV_8UC3, cv::Scalar(0, 0, 0));
    std::string fontPath = "./seguisym.ttf";

    // be careful to use the font that support Chinese
    cvx::CvxFont font(fontPath);
    if(!font.isLoad())return ;
    int i = 0, page = 0, x = 10, y = 10;
    std::vector<uint32_t> symbols;
    for (uint32_t c = 0x1f300; c <= 0x1f6cf; ++c) {
        symbols.push_back(c);
        if ((i + 1) % 16 == 0) {
            if ((i + 1) % 32 == 0) {
                cvx::putSymbols(img, symbols, cv::Point(x, y), font, 32, cv::Scalar(0,255,0));//randomColor(rng));
                y += 50;
                x = 10;
                symbols.clear();
            }
            else {
                cvx::putSymbols(img, symbols, cv::Point(x, y), font, 32, cv::Scalar(0,255,0));//randomColor(rng));
                x += 600;
                symbols.clear();
            }
        }
        if ((i + 1) % (14 * 32) == 0) {
            cv::imshow("test2", img);
            cv::Mat black(700, 1200, CV_8UC3, cv::Scalar(0, 0, 0));
            img = black;
            char k = cv::waitKey();
            x = 10, y = 10;
            if (k == 'q') {  // press q to quite
                return;
            }
        }
        i++;
    }

    cv::imshow("test2", img);
    cv::waitKey();
}
int main(int argc, char *argv[]) {
    if (!getenv("DISPLAY"))setenv("DISPLAY", "localhost:10.0", 1);
    testChinese();
    testSymbols();
    return 0;
}