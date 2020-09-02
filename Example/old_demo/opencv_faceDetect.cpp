//
// Created by caesar kekxv on 2020/5/20.
//

#include <opencv_tool.h>
#include <logger.h>

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
    logger::instance()->init_default();
    logger::instance()->d(TAG, __LINE__, "running go");

    opencv_tool opencvTool(logger::get_local_path() + "/data/haarcascades/haarcascade_frontalface_default.xml");
    logger::instance()->d(TAG, __LINE__, "opencvTool load %s", opencvTool.isLoad() ? "True" : "False");

    cv::VideoCapture cap;
    cap.open(0);
    int fps = (int) (1000 / cap.get(CAP_PROP_FPS));
    while (cap.isOpened()) {
        Mat frame;
        cap >> frame;
        opencv_tool::rotate180(frame);

        vector<Rect> faces;
        opencvTool.findMaxFace(frame,faces);
        for(const auto& item : faces){
            cv::rectangle(frame, item, Scalar(255, 255, 255),2, LINE_8,0);
        }

        imshow("frame", frame);
        if (27 == waitKey(fps)) {
            break;
        }
    }

    logger::instance()->d(TAG, __LINE__, "running finish");
    return 0;
}

