//
// Created by caesar kekxv on 2020/3/12.
//

#include <opencv2/opencv.hpp>
#include <logger.h>
#include <ctime>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    logger::instance()->init_default();
    // Start default camera
    VideoCapture video(0);

    // With webcam get(CV_CAP_PROP_FPS) does not work.
    // Let's see for ourselves.

    double fps = video.get(CAP_PROP_FPS);

//    video.set(cv::CAP_PROP_FRAME_WIDTH, 10000);
//    video.set(cv::CAP_PROP_FRAME_HEIGHT, 10000);
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
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_AUTO_WB         \t : %f", video.get(CAP_PROP_AUTO_WB));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_WB_TEMPERATURE  \t : %f",
                          video.get(CAP_PROP_WB_TEMPERATURE));
    logger::instance()->i(__FILENAME__, __LINE__, "CAP_PROP_FPS             \t : %f", fps);

    // Number of frames to capture
    int num_frames = 30;

    // Start and end times
    time_t start, end;

    // Variable for storing video frames
    Mat frame;

    logger::instance()->i(__FILENAME__, __LINE__, "Capturing %d frames", num_frames);


    // Start time
    time(&start);

    // Grab a few frames
    for (int i = 0; i < num_frames; i++) {
        video >> frame;
    }

    // End Time
    time(&end);

    // Time elapsed
    double seconds = difftime(end, start);
    logger::instance()->i(__FILENAME__, __LINE__, "Time taken             \t : %f seconds", seconds);

    // Calculate frames per second
    fps = num_frames / seconds;
    logger::instance()->i(__FILENAME__, __LINE__, "Estimated frames per second : %f", fps);

    // Release video
    video.release();
    return 0;
}