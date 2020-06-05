//
// Created by caesar kekxv on 2020/6/5.
//
#include <logger.h>
#include <kHttpd.h>
#include <kHttpdClient.h>
#include <opencv2/opencv.hpp>
#include <json11/json11.hpp>
#include <nlohmann/json.hpp>
#include <Base64/base64.h>

using namespace std;
using namespace cv;

vector<Vec3f> FindChess(const Mat &srcImage, Mat &image_copy) {
    //【1】载入原始图和Mat变量定义
    Mat midImage, dstImage;//临时变量和目标图的定义
    //【2】显示原始图
    Mat image_cut = Mat(srcImage);      //从img中按照rect进行切割，此时修改image_cut时image中对应部分也会修改，因此需要copy
    image_copy = image_cut.clone();   //clone函数创建新的图片
    //【3】转为灰度图，进行图像平滑
    cvtColor(image_copy, midImage, COLOR_BGR2GRAY);//灰度化
    //二值化
    threshold(midImage, midImage, 128, 255, THRESH_BINARY | THRESH_OTSU);
    //使用3*3内核来降噪
    blur(midImage, midImage, Size(3, 3));//进行模糊
    //GaussianBlur(midImage, midImage, Size(9, 9), 2, 2);
    //【4】进行霍夫圆变换
    vector<Vec3f> circles;
    //  HoughCircles(midImage, circles, CV_HOUGH_GRADIENT, 1.5, 10, 200, 100, 0, 0);
    //      第五个参数   圆的圆心之间的最小距离
    HoughCircles(midImage, circles, HOUGH_GRADIENT, 1.5, 35, 100, 25, 36, 43);

    //【5】依次在图中绘制出圆
    for (auto &i : circles) {
        Point center(cvRound(i[0]), cvRound(i[1]));
        int radius = cvRound(i[2]);
        //绘制圆心
        circle(midImage, center, 3, Scalar(0, 255, 0), -1, 8, 0);
        //绘制圆轮廓
        circle(midImage, center, radius, Scalar(155, 50, 255), 3, 8, 0);
        //绘制圆心
        circle(image_copy, center, 3, Scalar(0, 255, 0), -1, 8, 0);
        //绘制圆轮廓
        circle(image_copy, center, radius, Scalar(155, 50, 255), 3, 8, 0);
    }
    return circles;
}

int main(int argc, char *argv[]) {
    // Mat outImg;
    // vector<Vec3f> d = FindChess(imread("3.jpg"),outImg);
    // return 1;
    logger::instance()->init_default();
    kHttpd::Init();
    int thread_num = 20;
    auto web_root =
            logger::get_local_path() + logger::path_split + ".." + logger::path_split + "Example" + logger::path_split +
            "WebRoot";
    kHttpd kChess(web_root.c_str(), thread_num);

    kChess.set_cb("POST", "/FindChess",
                  [](void *kClient, const std::vector<unsigned char> &data, const std::string &url_path,
                     const std::string &method, int type, void *arg) -> int {
                      if (-1 != type)return -1;
                      auto *kHttpdClient = (::kHttpdClient *) kClient;
                      json11::Json json;
                      std::string err;
                      json = json11::Json::parse((const char *) data.data(), err);
                      if (!err.empty() || !json["image"].is_string()) {
                          string _ = "{\"error\":-1}";
                          kHttpdClient->ResponseContent.insert(kHttpdClient->ResponseContent.end(), _.begin(), _.end());
                          return 0;
                      }

                      string image = json["image"].string_value();
                      auto _index = image.find(',');
                      if (_index != string::npos) {
                          image = image.substr(_index + 1);
                      }
                      auto img_bin = base64_decode(image);
                      Mat img = imdecode(img_bin, 1);
                      if (img.empty()) {
                          string _ = "{\"error\":-2}";
                          kHttpdClient->ResponseContent.insert(kHttpdClient->ResponseContent.end(), _.begin(), _.end());
                          return 0;
                      }
                      nlohmann::json Json;
                      Json["error"] = 0;

                      Mat outImg;
                      auto out = FindChess(img, outImg);
                      nlohmann::json outJson;
                      for (auto &i : out) {
                          outJson.push_back({
                                                    {"x", i[0]},
                                                    {"y", i[1]},
                                                    {"r", i[2]},
                                            });
                      }
                      Json["data"] = outJson;
                      vector<unsigned char> photo;
                      vector<int> compression_params;
                      compression_params.push_back(IMWRITE_JPEG_QUALITY);
                      compression_params.push_back(85);
                      imencode(".jpg", outImg, photo, compression_params);
                      Json["image"] = "data:image/jpg;base64," + base64_encode(photo.data(), photo.size());

                      string _json = Json.dump();
                      kHttpdClient->ResponseContent.insert(kHttpdClient->ResponseContent.end(), _json.begin(),
                                                           _json.end());
                      return 0;
                  });

    kChess.listen(20, 8080, "0.0.0.0");
    return 0;
}