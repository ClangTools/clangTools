//
// Created by caesar kekxv on 2020/2/26.
//
#ifdef ENABLE_OPENCV

#include <opencv_tool.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <logger.h>

using namespace std;
using namespace cv;
using namespace std;


int main1(int argc, char **argv) {
    logger::instance()->init_default();
    logger::instance()->d(__FILENAME__, __LINE__, "running");
    // 读取RBG图片，转成Lab模式
    Mat bgr_image =
//            cv::imread(argc == 2 ? argv[1] : "../images/4.jpg");
            cv::imread(argc == 2 ? argv[1] : "../images/2.bmp");
    if (!bgr_image.rows) {
        cout << "imread failed!" << endl;
        return 0;
    }
    auto startTime = logger::get_time_tick();
    Mat lab_image;
    cvtColor(bgr_image, lab_image, COLOR_BGR2Lab);
    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
    // 提取L通道
    vector<Mat> lab_planes(3);
    split(lab_image, lab_planes);
    // CLAHE 算法
    Ptr<CLAHE> clahe = createCLAHE();
    clahe->setClipLimit(4);
    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
    Mat dst;
    clahe->apply(lab_planes[0], dst);
    dst.copyTo(lab_planes[0]);
    merge(lab_planes, lab_image);
    //恢复RGB图像
    Mat image_clahe;
    cvtColor(lab_image, image_clahe, COLOR_Lab2BGR);
    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
    //打印结果
    imshow("src", bgr_image);
    imshow("CLAHE", image_clahe);
    waitKey();
    logger::instance()->d(__FILENAME__, __LINE__, "end");
    return 0;
}

int main(int argc, char *argv[]) {
    logger::instance()->init_default();
    logger::instance()->d(__FILENAME__, __LINE__, "running");

    Mat gray;
    cv::Mat srcImage =
            cv::imread(argc == 2 ? argv[1] : "../images/2.bmp");
//            cv::imread(argc == 2 ? argv[1] : "../images/1.bmp");
    if (srcImage.empty())
        return -1;

    auto startTime = logger::get_time_tick();
    srcImage = srcImage(Rect(10, 10, srcImage.size().width - 10 * 2, srcImage.size().height - 10 * 2));


    if (srcImage.type() != CV_8U) {
        cvtColor(srcImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = srcImage.clone();
    }

    Mat gOut, out, mOut;

    /**
    int gSize = srcImage.size().height / 25;
    gSize = gSize > 3 ? gSize : 3;
    if (gSize % 2 == 0) {
        gSize += 1;
    }
    */
    int gSize = 5;
    // 去噪
    GaussianBlur(srcImage, gOut, Size(gSize, gSize), 1);   //高斯滤波
//    cv::medianBlur(srcImage, mOut, 5);//中值滤波
//    if (mOut.type() != CV_8U) {
//        cvtColor(mOut, gray, cv::COLOR_BGR2GRAY);
//    } else {
//        gray = mOut.clone();
//    }
//    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
//    adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_GAUSSIAN_C,
//                      THRESH_BINARY, 13 * 2 + 1, 3);
//    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);

//    方法一3以下的版本
//    Canny(mOut, out, 10, 150);
//    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
    Canny(gOut, out, 10, 50);

//    int elementSize = 1;
    // 膨胀、腐蚀
//    Mat element1 = getStructuringElement(MORPH_RECT, Size(elementSize, elementSize));
//
//    Mat element2 = getStructuringElement(MORPH_RECT, Size(elementSize + 3, elementSize + 3));
    // 膨胀一次，让轮廓突出
//    erode(out, out, element1);
//    dilate(out, out, element2);
//    erode(out, out, element1);

//    imshow("mOut", mOut);
    imshow("Canny", out);
    //查找轮廓
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(out, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
    int max_area = srcImage.size().height / 20;
    max_area = max_area * max_area;
    std::vector<cv::RotatedRect> candidates;
    for (const auto &contour : contours) {
        auto rect = cv::minAreaRect(contour);

        if (rect.boundingRect().area() < max_area)
            continue;
        // 不符合尺寸条件判断
        if (rect.size.height > 5)
            candidates.push_back(rect);
    }
    std::vector<cv::RotatedRect> results;
    opencv_tool::nms(results, candidates);
    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
    logger::instance()->d(__FILENAME__, __LINE__, "results : %lu", results.size());
    for (const auto &rRect :results) {
        ///*
        Point2f vertices[4];
        rRect.points(vertices);
        for (int i = 0; i < 4; i++)
            line(srcImage, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 255), 2);
        //*/
        /*
        std::vector<cv::Mat> outImages;
        opencv_tool::WarpAffine(srcImage, outImages, rRect);
        logger::instance()->d(__FILENAME__, __LINE__, "WarpAffine : %lu", outImages.size());
        logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
        for (const auto& outImage:outImages) {
            cv::imshow("outImage", outImage);
            cv::waitKey(0);
        }
        //*/
    }


    logger::instance()->d(__FILENAME__, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
    cv::imshow("gray", gray);
    cv::imshow("src Image", srcImage);
    cv::waitKey(0);

    logger::instance()->d(__FILENAME__, __LINE__, "end");
    return 0;
}

#else
int main(int argc,char * argv[]){
    return 0;
}
#endif