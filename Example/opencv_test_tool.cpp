//
// Created by caesar kekxv on 2020/3/12.
//

#include <opencv2/opencv.hpp>
#include <opencv_tool.h>
#include <logger.h>

#if defined(LINUX) || defined(linux)
#ifdef ENABLE_X11
#include <X11/Xlib.h>
#endif
#endif

#ifdef __FILENAME__
const char *TAG = __FILENAME__;
#else
const char *TAG = "opencv_test_tool";
#endif

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    logger::instance()->init_default();
#if defined(LINUX) || defined(linux)
#ifdef ENABLE_X11
    setenv("DISPLAY", "localhost:10.0", 1);
    logger::instance()->i(TAG, __LINE__, "XOpenDisplay : %s", XOpenDisplay(nullptr) ? "True" : "False");
#endif
#endif
#if 1
    Mat img = imread("C:\\Users\\John\\Desktop\\1.jpeg"), outMat;
    resize(img, img, Size(480, 640));

    opencv_tool::makeSineWave(img,img);

    opencv_tool::maskTranslucent(img, outMat, img.cols / 5, img.cols / 5, img.rows / 5, img.rows / 5);


    imshow("img", img);
    imshow("outMat", outMat);
    waitKey();
#endif
#if 0
    if (true) {
        Mat img = imread("/Users/caesar/Desktop/csc.bmp");
        // Mat img = imread("/Users/caesar/Desktop/2.jpg");
        // Mat img = imread("/Users/caesar/Desktop/4.jpeg");
        double score = opencv_tool::variance_of_laplacian(img);
        logger::instance()->i(TAG, __LINE__, "score %lf", score);
        return 0;
    }
    // Start default camera
    VideoCapture video(0);
    int num_frames = 10;

    // Variable for storing video frames
    Mat frame;
    auto fps = (long long) video.get(CAP_PROP_FPS);
    // Grab a few frames
    for (int i = 0; i < 3; i++) {
        video >> frame;
        imshow("frame", frame);
        double score = opencv_tool::variance_of_laplacian(frame);
        logger::instance()->i(TAG, __LINE__, "score %lf", score);
        waitKey(1000 / fps);
    }
    // Grab a few frames
    for (int i = 0; i < num_frames; i++) {
        video >> frame;
        logger::instance()->i(TAG, __LINE__, "Capturing");
        imshow("frame", frame);
        double score = opencv_tool::variance_of_laplacian(frame);
        logger::instance()->i(TAG, __LINE__, "score %lf", score);
        waitKey();
    }

    // Release video
    video.release();
#endif
    return 0;
}