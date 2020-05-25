//
// Created by k on 2019/10/22.
//

#ifndef ELECTRONICDEVICE_VIDEOTOOL_H
#define ELECTRONICDEVICE_VIDEOTOOL_H
#include <string>
#include <vector>
struct VideoInfo {
    /**
     * 摄像头名
     */
    std::string name;
    /**
     * 摄像头路径
     */
    std::string path;
    /**
     * 摄像头 VID
     */
    std::string vid;
    /**
     * 摄像头 PID
     */
    std::string pid;
    /**
     * 摄像头 序号
     */
    int index;
};
#ifdef ENABLE_OPENCV

#include <opencv2/opencv.hpp>
#include <v4l2Tool.h>

#include <logger.h>

class VideoTool {
public:
    /**
     * 初始化
     * @param videoInfo 待使用的摄像头
     */
    explicit VideoTool(const VideoInfo& videoInfo);
    /**
     * 释放
     */
    ~VideoTool();

    /**
     * 开启摄像头
     * @return
     */
    bool Open();
    /**
     * 是否开启状态
     * @return
     */
    bool isOpen();
    /**
     * 关闭摄像头
     * @return
     */
    bool Close();

    cv::VideoCapture GetCap();
#ifndef __APPLE__
    /**
     * 获取摄像头列表
     * @return
     */
    static std::vector<VideoInfo> GetVideoLists();
#endif
private:
    /**
     * 摄像头 OpenCV 对象
     */
    cv::VideoCapture cap{};
    VideoInfo videoInfo;
};


#else

class VideoTool {

};
#endif
#endif //ELECTRONICDEVICE_VIDEOTOOL_H