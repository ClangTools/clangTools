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

int main(int argc, char *argv[]) {
    logger::instance()->init_default();

    string faceXml = logger::get_local_path() + "/data/haarcascades/haarcascade_frontalface_alt2.xml";
    opencv_tool::instance()->init(faceXml);
    logger::instance()->d(TAG, __LINE__, "CascadeClassifier load : %s",
                          (opencv_tool::instance()->isLoad() ? "true" : "false"));

    cv::Mat srcImage =
            cv::imread(argc == 2 ? argv[1] : "../images/east-out-patient.jpg");
    if (srcImage.empty())
        return -1;
    int size = 5;
    srcImage = srcImage(Rect(size, size, srcImage.size().width - size * 2, srcImage.size().height - size * 2));
    auto startTime = logger::get_time_tick();

    std::vector<cv::RotatedRect> candidates, faces;
    int ret;
    ret = opencv_tool::FindLine(srcImage, candidates, faces);
    if (ret > 0) {
        for (int i = 0; i < candidates.size(); ++i) {
            // cv::imshow("rect", srcImage(candidates[i]));
            // cv::waitKey();
            cv::rectangle(srcImage, candidates[i].boundingRect(), Scalar(0, 0, 255));
        }
        for (int i = 0; i < faces.size(); ++i) {
            // cv::imshow("rect", srcImage(candidates[i]));
            // cv::waitKey();
            cv::rectangle(srcImage, faces[i].boundingRect(), Scalar(255, 0, 0));
        }
    }
    logger::instance()->d(TAG, __LINE__, "use time : %lld", logger::get_time_tick() - startTime);
    cv::imshow("src Image", srcImage);
    cv::waitKey(0);
    return 0;
}

#else
int main(int argc,char * argv[]){
    return 0;
}
#endif