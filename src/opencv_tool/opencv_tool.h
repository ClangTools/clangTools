//
// Created by caesar kekxv on 2020/2/26.
//

#ifndef TOOLS_OPENCV_TOOL_H
#define TOOLS_OPENCV_TOOL_H
#ifdef ENABLE_OPENCV

#include <string>
#include <opencv2/opencv.hpp>

class opencv_tool {
public:
    static opencv_tool *instance();

    explicit opencv_tool(std::string faceXml = "");

    int init(const std::string &filename = "");

    ~opencv_tool();

    bool findMaxFace(const cv::Mat &inMat, std::vector<cv::Rect> &faces);

    cv::Mat findMaxFace(const cv::Mat &inMat);

    inline bool isLoad() {
        return !faceCascade.empty();
    }

public:

    template<typename T>
    static std::vector<int> argsort(const std::vector<T> &array);

// 实现argsort功能
    template<typename T>
    static std::vector<int> argsort(const std::vector<T> &array, const std::vector<T> &array2);

    /**
     * NMS 方法（Non Maximum Suppression，非极大值抑制）
     * @param boxes
     * @param overlapThresh
     * @return
     */
    static int
    nms(std::vector<cv::RotatedRect> &results, std::vector<cv::RotatedRect> boxes, float overlapThresh = 0.5);

    /**
     * 检测文本区域
     * @param srcImage
     * @return
     */
    static std::vector<cv::RotatedRect> FindLine_mserGetPlate(const cv::Mat &srcImage);

    static std::vector<cv::RotatedRect> FindLine_Gray(const cv::Mat &srcImage);

    /**
     * 检测文本区域
     * @param srcImage
     * @return
     */
    static int
    FindLine(const cv::Mat &srcImage, std::vector<cv::RotatedRect> &texts, std::vector<cv::RotatedRect> &faces,
             bool useMserGetPlate = false);

    static int WarpAffine(const cv::Mat &srcImage,std::vector<cv::Mat> &outImage,cv::RotatedRect box);

private:
    std::string faceXml = "./haarcascades/haarcascade_frontalface_alt2.xml";
    // string faceXml = "./haarcascades/haarcascade_frontalface_default.xml";
    cv::CascadeClassifier faceCascade;

private:
    static void rotate180(cv::OutputArray &inMat);
};


#endif //ENABLE_OpenCV
#endif //TOOLS_OPENCV_TOOL_H
