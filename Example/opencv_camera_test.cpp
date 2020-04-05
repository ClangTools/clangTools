//
// Created by caesar kekxv on 2020/3/12.
//

#include <opencv2/opencv.hpp>
#include <logger.h>
#include <ctime>

#if defined(LINUX) || defined(linux)
#ifdef ENABLE_X11
#include <X11/Xlib.h>
#endif
#endif

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    logger::instance()->init_default();
#if defined(LINUX) || defined(linux)
#ifdef ENABLE_X11
    setenv("DISPLAY", "localhost:10.0", 1);
    logger::instance()->i(__FILENAME__, __LINE__, "XOpenDisplay : %s", XOpenDisplay(nullptr) ? "True" : "False");
#endif
#endif
    // Start default camera
    VideoCapture video(0);
    // Number of frames to capture
    int num_frames = 5;

    // With webcam get(CV_CAP_PROP_FPS) does not work.
    // Let's see for ourselves.

    auto fps = (long long) video.get(CAP_PROP_FPS);


    video.set(cv::CAP_PROP_XI_HDR_T2, 1);
    video.set(cv::CAP_PROP_XI_HDR_T1, 1);
    video.set(cv::CAP_PROP_XI_HDR, 1);
//    video.set(cv::CAP_PROP_FRAME_WIDTH, 10000);
//    video.set(cv::CAP_PROP_FRAME_HEIGHT, 10000);
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_XI_HDR_T2       \t : %f", video.get(CAP_PROP_XI_HDR_T2));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_XI_HDR_T1       \t : %f", video.get(CAP_PROP_XI_HDR_T1));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_XI_HDR          \t : %f", video.get(CAP_PROP_XI_HDR));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_FRAME_WIDTH     \t : %f", video.get(CAP_PROP_FRAME_WIDTH));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_FRAME_HEIGHT    \t : %f", video.get(CAP_PROP_FRAME_HEIGHT));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_BRIGHTNESS      \t : %f", video.get(CAP_PROP_BRIGHTNESS));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_CONTRAST        \t : %f", video.get(CAP_PROP_CONTRAST));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_SATURATION      \t : %f", video.get(CAP_PROP_SATURATION));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_HUE             \t : %f", video.get(CAP_PROP_HUE));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_GAIN            \t : %f", video.get(CAP_PROP_GAIN));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_EXPOSURE        \t : %f", video.get(CAP_PROP_EXPOSURE));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_AUTO_EXPOSURE   \t : %f",
                          video.get(CAP_PROP_AUTO_EXPOSURE));
    // logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_AUTO_WB         \t : %f", video.get(CAP_PROP_AUTO_WB));
    // logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_WB_TEMPERATURE  \t : %f",
    //                       video.get(CAP_PROP_WB_TEMPERATURE));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_FPS             \t : %lld", fps);


    // Start and end times
    time_t start, end;

    // Variable for storing video frames
    Mat frame;

    logger::instance()->i(__FILENAME__, __LINE__, "Capturing %d frames", num_frames);


    // Start time
    start = logger::get_time_tick();

    // Grab a few frames
    for (int i = 0; i < num_frames; i++) {
        video >> frame;
    }

    // End Time
    end = logger::get_time_tick();

    // Time elapsed
    long long seconds = end - start;
    // Calculate frames per second
    fps = num_frames * 1000 / seconds;
    logger::instance()->i(__FILENAME__, __LINE__, "Time taken             \t : %.02lf seconds ; FPS:%lld",
                          ((double) seconds) / 1000, fps);


    logger::instance()->i(__FILENAME__, __LINE__, "Capturing");

    vector<Mat> img(3);
    vector<float> times(3);
    for (int i = 0; i < 3; i++) {
        video >> img[i];
        times[i] = 1 / (i / 3.);
        logger::instance()->i(__FILENAME__, __LINE__, "Capturing : %d", i);
    }

    Mat response;
    Ptr<CalibrateDebevec> calibrate = createCalibrateDebevec();
    calibrate->process(img, response, times);
    imshow("response", response);
    Mat hdr;
    Ptr<MergeDebevec> merge_debevec = createMergeDebevec();
    merge_debevec->process(img, hdr, times, response);
    imshow("hdr", hdr);

#if defined(LINUX) || defined(linux)
#ifdef ENABLE_X11
    logger::instance()->i(__FILENAME__, __LINE__, "XOpenDisplay : %s", XOpenDisplay(nullptr) ? "True" : "False");
    for (int i = 0; i < 3; i++) {
        imshow(to_string(i), img[i]);
        logger::instance()->i(__FILENAME__, __LINE__, "Capturing : %d", i);
    }
    waitKey();
#endif
#endif

    // Release video
    video.release();
    return 0;
}