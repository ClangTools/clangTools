//
// Created by k on 2019/10/22.
//

#include "VideoTool.h"

using namespace cv;
using namespace std;

#ifndef APPLE
using namespace clangTools;

/**
 * 获取摄像头列表
 * @return
 */
std::vector<VideoInfo> VideoTool::GetVideoLists() {
    std::vector<VideoInfo> videoList;
#ifdef WIN32
    v4l2Tool vTool;
    vTool.Init();
    auto devLen = vTool.NumDevices();
    auto dev = vTool.GetDevices();
    for (int i = 0; i < devLen; i++) {
        VideoInfo vInfo;
        vInfo.name = dev[i].GetFriendlyName();
        vInfo.index = dev[i].GetId() - 1;
        vInfo.vid = dev[i].vid;
        vInfo.pid = dev[i].pid;
        vInfo.path = "";
        videoList.push_back(vInfo);
    }
#else
    VideoDevice vd[25];
    int vdLen = 25;
    int ret = v4l2Tool::GetDevices(vd, vdLen);
    for (int i = 0; i < ret; i++) {
        VideoInfo vInfo;
        vInfo.name = vd[i].name;
        vInfo.path = vd[i].path;
        vInfo.vid = vd[i].vid;
        vInfo.pid = vd[i].pid;
        sscanf(vd[i].path, "%*[^0-9]%d", &(vInfo.index));
        videoList.push_back(vInfo);
    }
#endif
    return videoList;
}

#endif

/**
 * 初始化
 * @param videoInfo 待使用的摄像头
 */
VideoTool::VideoTool(const VideoInfo &videoInfo) {
    this->videoInfo.name = videoInfo.name;
    this->videoInfo.path = videoInfo.path;
    this->videoInfo.vid = videoInfo.vid;
    this->videoInfo.pid = videoInfo.pid;
    this->videoInfo.index = videoInfo.index;
}

/**
 * 释放
 */
VideoTool::~VideoTool() {
    Close();
}

/**
 * 是否开启状态
 * @return
 */
bool VideoTool::isOpen() {
    return cap.isOpened();
}

/**
 * 开启摄像头
 * @return
 */
bool VideoTool::Open() {
    Close();
    cap = VideoCapture();
#ifdef WIN32
    return cap.open(videoInfo.index);
#else
    return cap.open(videoInfo.path);
#endif
}

/**
 * 关闭摄像头
 * @return
 */
bool VideoTool::Close() {
    if (isOpen())
        cap.release();
    return cap.isOpened();
}

cv::VideoCapture VideoTool::GetCap() {
    return cap;
}
