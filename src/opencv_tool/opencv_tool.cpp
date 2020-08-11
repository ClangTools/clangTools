//
// Created by caesar kekxv on 2020/2/26.
//

//
// Created by caesar on 2019/9/21.
//
#ifdef ENABLE_OPENCV

#include "opencv_tool.h"
#include <logger.h>
#include "QrCode.hpp"


using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;

using namespace std;
using namespace cv;

#ifdef __FILENAME__
const char *opencv_tool::TAG = __FILENAME__;
#else
const char * opencv_tool::TAG = "opencv_tool";
#endif

opencv_tool opencvTool{};

opencv_tool *opencv_tool::instance() {
    return &opencvTool;
}

opencv_tool::opencv_tool(string faceXml) {
    if (faceXml.empty())
        faceXml = this->faceXml;
    faceCascade.load(faceXml);
}

opencv_tool::~opencv_tool() = default;


/*
bool opencv_tool::putText(cv::Mat &img, const std::string &text, cv::Point org,
                    int fontFace, double fontScale, cv::Scalar color,
                    int thickness, int lineType,
                    bool bottomLeftOrigin, const std::string file_path){
    if(file_path.empty()){

    }else{
        Ptr<freetype::FreeType2> ft2 = cv::freetype::createFreeType2();
        ft2->loadFontData(file_path, 0);
        Size textSize = ft2->getTextSize(text,
                                         fontHeight,
                                         thickness,
                                         &baseline);
        if(thickness > 0){
            baseline += thickness;
        }

        Point textOrg((img.cols - textSize.width) / 2,
                      (img.rows + textSize.height) / 2);
        rectangle(img, textOrg + Point(0, baseline),
                  textOrg + Point(textSize.width, -textSize.height),
                  Scalar(0,255,0),1,8);
        line(img, textOrg + Point(0, thickness),
             textOrg + Point(textSize.width, thickness),
             Scalar(0, 0, 255),1,8);
        ft2->putText(img, text, textOrg, fontHeight,
                     Scalar::all(255), thickness, linestyle, true );
        ft2.release();
    }

}
*/

bool opencv_tool::findMaxFace(const cv::Mat &inMat, std::vector<cv::Rect> &faces) {
    if (faceCascade.empty())return false;
    if (inMat.empty())return false;

    cv::Mat img_gray;
    if (inMat.type() != CV_8U) {
        cvtColor(inMat, img_gray, cv::COLOR_BGR2GRAY);
    } else {
        img_gray = inMat;
    }
    cv::equalizeHist(img_gray, img_gray);

    faceCascade.detectMultiScale(img_gray, faces,
                                 1.1,
                                 3,
                                 cv::CASCADE_SCALE_IMAGE
            // ,cv::Size(30, 30)
    );
    /*
    if (faces.empty()) {
        rotate180(inMat);
        rotate180(img_gray);
        faceCascade.detectMultiScale(img_gray, faces, 1.1, 3, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
    }
    */
    return (!faces.empty());
}

cv::Mat opencv_tool::findMaxFace(const cv::Mat &inMat) {
    std::vector<cv::Rect> faces;
    if (!findMaxFace(inMat, faces))return cv::Mat();
    cv::Rect maxFace = faces[0];
    for (size_t i = 1; i < faces.size(); i++) {
        if (faces[i].height * faces[i].width > maxFace.width + maxFace.height) {
            maxFace = faces[i];
        }
    }

    int x0 = maxFace.x, y0 = maxFace.y, w = maxFace.width, h = maxFace.height;

    int x1 = x0 + (w / 2), y1 = y0 + (w / 2);
    int w1 = (int) (w * 3 / 2), h1 = h * 2;
    int x2 = x1 - (w1 / 2), y2 = y1 - (h1 / 2);
    if (x2 < 0)x2 = 0;
    if (y2 < 0)y2 = 0;
    if (x2 + w1 >= inMat.size().width)w1 = inMat.size().width - x2 - 1;
    if (y2 + h1 >= inMat.size().height)h1 = inMat.size().height - y2 - 1;

    maxFace = cv::Rect(x2, y2, w1, h1);

    return inMat(maxFace);
}

int opencv_tool::init(const string &filename) {
    faceCascade.load(filename);
    return !faceCascade.empty();
}

void opencv_tool::rotate180(cv::OutputArray &inMat) {
    cv::flip(inMat, inMat,
             0);
    cv::flip(inMat, inMat, 1);
}

// 实现argsort功能
template<typename T>
std::vector<int> opencv_tool::argsort(const std::vector<T> &array) {
    const int array_len(array.size());
    std::vector<int> array_index(array_len, 0);
    for (int i = 0; i < array_len; ++i)
        array_index[i] = i;

    std::sort(array_index.begin(), array_index.end(),
              [&array](int pos1, int pos2) { return (array[pos1] < array[pos2]); });

    return array_index;
}

// 实现argsort功能
template<typename T>
std::vector<int> opencv_tool::argsort(const std::vector<T> &array, const std::vector<T> &array2) {
    if (array.size() != array2.size())throw exception();
    const int array_len(array.size());
    std::vector<int> array_index(array_len, 0);
    for (int i = 0; i < array_len; ++i)
        array_index[i] = i;

    std::sort(array_index.begin(), array_index.end(),
              [&array, &array2](int pos1, int pos2) {
                  if (array[pos1] < array[pos2] && array2[pos1] < array2[pos2])return true;
                  else if (array[pos1] > array[pos2] && array2[pos1] > array2[pos2])return false;
                  auto _1 = abs(array[pos1] - array[pos2]);
                  auto _2 = abs(array2[pos1] - array2[pos2]);
                  if (array[pos1] > array[pos2] && array2[pos1] < array2[pos2]) {
                      return _2 > _1;
                  } else {
                      return _2 < _1;
                  }
                  // return (array[pos1] < array[pos2]);
              });

    return array_index;
}

/**
 * NMS 方法（Non Maximum Suppression，非极大值抑制）
 * @param boxes
 * @param overlapThresh
 * @return
 */
int opencv_tool::nms(vector<RotatedRect> &results, std::vector<RotatedRect> boxes, float overlapThresh) {
    if (boxes.empty())return -1;

    vector<int> pick;

    //# 取四个坐标数组
    vector<int> x1;
    vector<int> y1;
    vector<int> x2;
    vector<int> y2;
    vector<int> area;
    for (const auto &item : boxes) {
        auto rect = item.boundingRect();
        x1.push_back(rect.x);
        y1.push_back(rect.y);
        x2.push_back(rect.x + rect.width);
        y2.push_back(rect.y + rect.height);
        //# 计算面积数组
        area.push_back(rect.area());
    }

    // # 按得分排序（如没有置信度得分，可按坐标从小到大排序，如右下角坐标）
    // vector<int> idxs = argsort(y2);
    vector<int> idxs = argsort(x2, y2);

    //# 开始遍历，并删除重复的框
    while (!idxs.empty()) {
        //# 将最右下方的框放入pick数组
        auto last = idxs.size() - 1;
        int i = idxs[last];
        pick.push_back(i);


        vector<int> xx1;
        vector<int> yy1;
        vector<int> xx2;
        vector<int> yy2;
        vector<int> w;
        vector<int> h;
        vector<int> overlap;
        vector<int> delete_index;
        // Mat srcImage = _srcImage.clone();
        // cv::rectangle(srcImage, boxes[i].boundingRect(), Scalar(0, 0, 255), 1, LINE_8, 0);
        // imshow("srcImage",srcImage);
        // waitKey();
        //# 找剩下的其余框中最大坐标和最小坐标
        for (auto j = 0; j < idxs.size() - 1; j++) {
            auto index = idxs[j];
            xx1.push_back(x1[i] > x1[index] ? x1[i] : x1[index]);
            yy1.push_back(y1[i] > y1[index] ? y1[i] : y1[index]);
            xx2.push_back(x2[i] < x2[index] ? x2[i] : x2[index]);
            yy2.push_back(y2[i] < y2[index] ? y2[i] : y2[index]);
            int _w = xx2[xx2.size() - 1] - xx1[xx1.size() - 1] + 1;
            int _h = yy2[yy2.size() - 1] - yy1[yy1.size() - 1] + 1;
            _w = _w > 0 ? _w : 0;
            _h = _h > 0 ? _h : 0;
            w.push_back(_w);
            h.push_back(_h);
            float _overlap = float(_w * _h) / area[index];
            overlap.push_back(_overlap);
            if (_overlap > overlapThresh) {
                delete_index.push_back(j);
            }
        }
        delete_index.push_back(last);
        std::sort(delete_index.begin(), delete_index.end(), [](int y, int _y) -> bool {
            return y > _y;
        });
        auto iter = unique(delete_index.begin(), delete_index.end());
        delete_index.erase(iter, delete_index.end());

        for (auto _index : delete_index) {
            idxs.erase(idxs.begin() + _index);
        }
    }
    for (auto _index : pick) {
        results.push_back(boxes[_index]);
    }
    // return boxes[pick].astype("int");
    return results.size();
}


std::vector<cv::RotatedRect> opencv_tool::FindLine_mserGetPlate(const cv::Mat &srcImage) {
    // HSV空间转换
    cv::Mat gray, gray_neg;

    if (srcImage.type() != CV_8U) {
        cvtColor(srcImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = srcImage.clone();
    }
    // 灰度转换
    // cv::cvtColor(srcImage, gray, COLOR_BGR2GRAY);
    // 取反值灰度
    gray_neg = 255 - gray;
    std::vector<std::vector<cv::Point> > regContours;
    std::vector<std::vector<cv::Point> > charContours;

    // 创建MSER对象
    cv::Ptr<cv::MSER> mesr1 = cv::MSER::create(2, 10, 5000, 0.5, 0.5);
    cv::Ptr<cv::MSER> mesr2 = cv::MSER::create(2, 2, 400, 0.1, 0.3);


    std::vector<cv::Rect> bboxes1;
    std::vector<cv::Rect> bboxes2;
    // MSER+ 检测
    mesr1->detectRegions(gray, regContours, bboxes1);
    // MSER-操作
    mesr2->detectRegions(gray_neg, charContours, bboxes2);

    cv::Mat mserMapMat = cv::Mat::zeros(srcImage.size(), CV_8UC1);
    cv::Mat mserNegMapMat = cv::Mat::zeros(srcImage.size(), CV_8UC1);

    for (int i = (int) regContours.size() - 1; i >= 0; i--) {
        // 根据检测区域点生成mser+结果
        const std::vector<cv::Point> &r = regContours[i];
        for (const auto &pt : r) {
            mserMapMat.at<unsigned char>(pt) = 255;
        }
    }
    // MSER- 检测
    for (int i = (int) charContours.size() - 1; i >= 0; i--) {
        // 根据检测区域点生成mser-结果
        const std::vector<cv::Point> &r = charContours[i];
        for (const auto &pt : r) {
            mserNegMapMat.at<unsigned char>(pt) = 255;
        }
    }
    // mser结果输出
    cv::Mat mserResMat;
    // mser+与mser-位与操作
    mserResMat = mserMapMat & mserNegMapMat;
    // 闭操作连接缝隙
    cv::Mat mserClosedMat;
    cv::morphologyEx(mserResMat, mserClosedMat,
                     cv::MORPH_CLOSE, cv::Mat::ones(1, 20, CV_8UC1));
    // cv::imshow("mserClosedMat", mserClosedMat);
    // 寻找外部轮廓
    std::vector<std::vector<cv::Point> > plate_contours;
    cv::findContours(mserClosedMat, plate_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    // 候选车牌区域判断输出
    std::vector<cv::RotatedRect> candidates;
    int max_area = srcImage.size().height / 25;
    max_area = max_area * max_area;
    for (size_t i = 0; i != plate_contours.size(); ++i) {
        //计算当前轮廓的面积
        double area = contourArea(plate_contours[i]);
        //面积小于1000的全部筛选掉
        if (area < max_area)
            continue;
        // 求解最小外界矩形
        auto rect = cv::minAreaRect(plate_contours[i]);
        // 不符合尺寸条件判断
        if (rect.size.height > 5)
            candidates.push_back(rect);
    }
    return candidates;
}

std::vector<cv::RotatedRect> opencv_tool::FindLine_Gray(const cv::Mat &srcImage) {
    // HSV空间转换
    cv::Mat gray;
    // 灰度转换
    // cv::cvtColor(srcImage, gray, COLOR_BGR2GRAY);

    if (srcImage.type() != CV_8U) {
        cvtColor(srcImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = srcImage.clone();
    }

    // # ret, gray = cv2.threshold(gray, 200, 255, cv2.THRESH_BINARY)
    GaussianBlur(gray, gray, Size(5, 5), 0);

    adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_MEAN_C,
                      THRESH_BINARY, 13, 15);

    // 膨胀、腐蚀
    Mat element1 = getStructuringElement(MORPH_RECT, Size(10, 2));
    Mat element2 = getStructuringElement(MORPH_RECT, Size(8, 3));

    // 膨胀一次，让轮廓突出
    erode(gray, gray, element2);

    // 腐蚀一次，去掉细节
    dilate(gray, gray, element1);

    // 再次膨胀，让轮廓明显一些
    erode(gray, gray, element2, Point(-1, -1), 2);

//    logger::instance()->d(TAG, __LINE__, "imshow GaussianBlur");
//    imshow("GaussianBlur", gray);

//    Mat vis = srcImage.clone();
//    Mat nmsImg = srcImage.clone();

    // # 调用 MSER 算法
    auto mser = MSER::create(2, 200);
    // 获取文本区域
    std::vector<cv::Rect> _;
    std::vector<std::vector<cv::Point> > regions;
    // MSER+ 检测
    mser->detectRegions(gray, regions, _);
    vector<vector<Point> > hulls(regions.size());
    std::vector<RotatedRect> keep;
    for (size_t i = 0; i < regions.size(); i++) {
        convexHull(regions[i], hulls[i]);
        auto rect = minAreaRect(hulls[i]);
        keep.push_back(rect);
        // cv::rectangle(vis, rect.boundingRect(), Scalar(255, 255, 0), 1);
    }
//    logger::instance()->d(TAG, __LINE__, "imshow vis");
//    imshow("vis", vis);

//    Mat img = srcImage.clone();
//    polylines(img, hulls, true, Scalar(0, 255, 0));
//    logger::instance()->d(TAG, __LINE__, "imshow img");
//    imshow("img", img);

    return keep;
}


int opencv_tool::FindLine(const cv::Mat &srcImage, std::vector<cv::RotatedRect> &texts,
                          std::vector<cv::RotatedRect> &faces, bool useMserGetPlate) {
    // 候选车牌区域检测
    std::vector<cv::RotatedRect> candidates;
    if (useMserGetPlate) {
        candidates = FindLine_mserGetPlate(srcImage);
    } else {
        candidates = FindLine_Gray(srcImage);
    }
    std::vector<cv::Rect> _faces;
    opencv_tool::instance()->findMaxFace(srcImage.clone(), _faces);
    for (const auto &item : _faces) {
        Point2f _center(item.x + item.width / 2, item.y + item.height / 2);
        Size _size(item.width * 3 / 2, item.height * 2);
        float _angle = 0;
        // cv::RotatedRect face(_center, _size, _angle);
        candidates.emplace_back(_center, _size, _angle);
        faces.emplace_back(_center, _size, _angle);
    }
    int ret = nms(texts, candidates, 0.5);
    if (!faces.empty()) {
        for (int i = texts.size() - 1; i >= 0; i--) {
            for (const auto &face : faces) {
                if (face.center == texts[i].center) {
                    texts.erase(texts.begin() + i);
                    break;
                }
                auto rect = face.boundingRect();
                if (
                        (texts[i].center.x > rect.x && texts[i].center.x < rect.x + rect.width)
                        && (texts[i].center.y > rect.y && texts[i].center.y < rect.y + rect.height)
                        ) {
                    texts.erase(texts.begin() + i);
                    break;
                }
            }

        }
    }
    return ret;
}

int opencv_tool::WarpAffine(const cv::Mat &srcImage, std::vector<cv::Mat> &outImage, cv::RotatedRect rRect) {
    Point2f vertices[4];
    rRect.points(vertices);
    ////获取四个顶点坐标，然后放到容器中，其实就是构造一个伪“contours”
    vector<vector<Point> > co_ordinates;
    co_ordinates.emplace_back();
    co_ordinates[0].push_back(vertices[0]);
    co_ordinates[0].push_back(vertices[1]);
    co_ordinates[0].push_back(vertices[2]);
    co_ordinates[0].push_back(vertices[3]);
    Mat mask1 = Mat::zeros(srcImage.size(), CV_8UC1);
    //图片区域绘制成白色的
    drawContours(mask1, co_ordinates, 0, Scalar(255), FILLED, 8);

    Mat out1;
    //通过模板复制，其实网上有很多方法，我觉得这个最简单。
    srcImage.copyTo(out1, mask1);

    //图片的旋转中心以及旋转角度
    Point2f center = rRect.center;
    float angle = rRect.angle;

    //旋转角度调整
    if (90 <= angle && angle < 45) {
        angle -= 90;
    } else if (-90 <= angle && angle < -45) {
        angle += 90;
    }

    //得到变换矩阵
    Mat rotate_matrix = getRotationMatrix2D(center, angle, 1);

    //旋转图像
    warpAffine(out1, out1, rotate_matrix, out1.size(), 1, 0, Scalar(0));//仿射变换
    //剪切矫正部分
    vector<vector<Point>> contours01;
    vector<Vec4i> hierarchy01;
    Mat gray_temp;
    cvtColor(out1, gray_temp, COLOR_BGR2GRAY);
    findContours(gray_temp, contours01, hierarchy01, RETR_LIST, CHAIN_APPROX_SIMPLE);


    int max_area = srcImage.size().height / 20;
    max_area = max_area * max_area;
    std::vector<cv::RotatedRect> candidates;
    for (const auto &contour : contours01) {
        auto rect = cv::minAreaRect(contour);

        if (rect.boundingRect().area() < max_area)
            continue;
        // 不符合尺寸条件判断
        if (rect.size.height > 5)
            candidates.push_back(rect);
    }
    std::vector<cv::RotatedRect> results;
    opencv_tool::nms(results, candidates);

    for (auto &result : results) {
        //获取剪切矩形
        Rect rect = result.boundingRect();
        if (rect.x < 0)rect.x = 0;
        if (rect.y < 0)rect.y = 0;
        if (rect.x + rect.width >= out1.size().width)rect.width = out1.size().width - rect.x - 1;
        if (rect.y + rect.height >= out1.size().height)rect.height = out1.size().height - rect.y - 1;
        outImage.push_back(out1(rect));
    }
    return outImage.size();
}

void opencv_tool::create2dArray(Mat *input, uint8_t ***outputPtr) {
    int imgWidth = input->cols;
    int imgHeight = input->rows;

    (*outputPtr) = new uint8_t *[imgHeight];
    for (int i = 0; i < imgHeight; ++i) {
        (*outputPtr)[i] = new uint8_t[imgWidth];
    }
    for (int i = 0; i < imgHeight; i++) {
        for (int j = 0; j < imgWidth; j++) {
            (*outputPtr)[i][j] = input->at<uint8_t>(i, j);
        }
    }
}

/*---------------------------------------------------------------------------*/
uint8_t opencv_tool::saturated_add(uint8_t val1, int8_t val2) {
    int16_t val1_int = val1;
    int16_t val2_int = val2;
    int16_t tmp = val1_int + val2_int;

    if (tmp > 255) {
        return 255;
    } else if (tmp < 0) {
        return 0;
    } else {
        return tmp;
    }
}

/*---------------------------------------------------------------------------*/
void opencv_tool::ImgDithering(const cv::Mat &gray, std::vector<std::vector<unsigned char>> &outputPtr) {
    Mat dithImg = gray.clone();

    if (dithImg.type() != CV_8U) {
        cvtColor(dithImg, dithImg, cv::COLOR_BGR2GRAY);
    }

    /* Get the size info */
    int imgWidth = dithImg.cols;
    int imgHeight = dithImg.rows;
    logger::instance()->d(TAG, __LINE__, "Image width : %d", imgWidth);
    logger::instance()->d(TAG, __LINE__, "Image height: %d", imgHeight);

    outputPtr = std::vector<std::vector<unsigned char>>(imgHeight, std::vector<unsigned char>(imgWidth));

    /* Run the 'Floyd-Steinberg' dithering algorithm ... */
    int err;
    int8_t a, b, c, d;

    for (int i = 0; i < imgHeight; i++) {
        for (int j = 0; j < imgWidth; j++) {
            if (dithImg.at<uint8_t>(i, j) > 127) {
                err = dithImg.at<uint8_t>(i, j) - 255;
                dithImg.at<uint8_t>(i, j) = 255;
                outputPtr[i][j] = 255;
            } else {
                err = dithImg.at<uint8_t>(i, j) - 0;
                dithImg.at<uint8_t>(i, j) = 0;
                outputPtr[i][j] = 0;
            }

            a = (err * 7) / 16;
            b = (err * 1) / 16;
            c = (err * 5) / 16;
            d = (err * 3) / 16;

            if ((i != (imgHeight - 1)) && (j != 0) && (j != (imgWidth - 1))) {
                dithImg.at<uint8_t>(i + 0, j + 1) = saturated_add(dithImg.at<uint8_t>(i + 0, j + 1), a);
                dithImg.at<uint8_t>(i + 1, j + 1) = saturated_add(dithImg.at<uint8_t>(i + 1, j + 1), b);
                dithImg.at<uint8_t>(i + 1, j + 0) = saturated_add(dithImg.at<uint8_t>(i + 1, j + 0), c);
                dithImg.at<uint8_t>(i + 1, j - 1) = saturated_add(dithImg.at<uint8_t>(i + 1, j - 1), d);
            }
        }
    }
}

cv::Mat opencv_tool::CreateQrCode(const std::string &data, int size) {
    const QrCode qr = QrCode::encodeText(
            data.c_str(),
            QrCode::Ecc::LOW);
    cv::Mat qrImg(qr.getSize() * size, qr.getSize() * size, CV_8UC3,
                  cv::Scalar(255, 255, 255)); // create a black background


    for (int y = 0; y < qr.getSize(); y++) {
        for (int x = 0; x < qr.getSize(); x++) {
            if (qr.getModule(x, y)) {
                // for (int i = 0; i < 3; i++)
                {
                    for (int _1 = 0; _1 < size; _1++) {
                        for (int _2 = 0; _2 < size; _2++) {
                            // qrImg.at<uint8_t>(x * size + _1, y * size + _2, i) = 0;
                            auto &pixel = qrImg.at<Vec3b>(x * size + _1, y * size + _2);
                            pixel[0] = 0;
                            pixel[1] = 0;
                            pixel[2] = 0;
                        }
                    }
                }
            } else {
                // for (int i = 0; i < 3; i++)
                {
                    for (int _1 = 0; _1 < size; _1++) {
                        for (int _2 = 0; _2 < size; _2++) {
                            // qrImg.at<uint8_t>(x * size + _1, y * size + _2, i) = 255;
                            auto &pixel = qrImg.at<Vec3b>(x * size + _1, y * size + _2);
                            pixel[0] = 255;
                            pixel[1] = 255;
                            pixel[2] = 255;
                        }
                    }
                }
            }
        }
    }
    return qrImg;
}

/**
 * 基于OpenCV的图像模糊与否检测
 * https://blog.csdn.net/tianzhaixing2013/article/details/77746653
 * @param img
 * @return
 */
double opencv_tool::variance_of_laplacian(const Mat &img) {
    if (img.empty()) {
        return 0.0;
    }
    Mat image;
    resize(img, image, Size(320, 240));
    const int imgrows = image.size().height;
    const int imgcols = image.size().width;
    Mat gray(Size(imgcols, imgrows), CV_8UC1, 1);
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, COLOR_BGR2GRAY);
    } else if (image.channels() == 1) {
        cv::copyTo(image, gray, NULL);
    } else {
        cv::cvtColor(image, gray, COLOR_BGRA2GRAY);
    }

    Mat lap(Size(imgcols, imgrows), CV_64FC1, 1);
    cv::Laplacian(gray, lap, 3);

    Scalar lap_mean, lap_stddev;
    cv::meanStdDev(lap, lap_mean, lap_stddev);
    return lap_stddev.val[0];
}

int opencv_tool::maskTranslucent(const Mat &inMat, Mat &outMat, int left, int right, int top, int bottom,
                                 cv::Scalar color) {
    Mat img = inMat.clone();
    int w = img.size().width, h = img.size().height;
    double alphaReserve = 0.0;
    unsigned char BChannel = color[0],
            GChannel = color[1],
            RChannel = color[2];

    for (int row = 0; row < top; row++) {
        alphaReserve = row / (double) top;
        for (int col = 0; col < w; col++) {
            img.at<Vec3b>(row, col)[0] = img.at<Vec3b>(row, col)[0] * alphaReserve + BChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[1] = img.at<Vec3b>(row, col)[1] * alphaReserve + GChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[2] = img.at<Vec3b>(row, col)[2] * alphaReserve + RChannel * (1 - alphaReserve);
        }
    }
    for (int _row = 0; _row < bottom; _row++) {
        alphaReserve = _row / (double) bottom;
        int row = h - _row - 1;
        for (int col = 0; col < w; col++) {
            img.at<Vec3b>(row, col)[0] = img.at<Vec3b>(row, col)[0] * alphaReserve + BChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[1] = img.at<Vec3b>(row, col)[1] * alphaReserve + GChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[2] = img.at<Vec3b>(row, col)[2] * alphaReserve + RChannel * (1 - alphaReserve);
        }
    }
    for (int col = 0; col < left; col++) {
        alphaReserve = col / (double) left;
        for (int row = 0; row < h; row++) {
            img.at<Vec3b>(row, col)[0] = img.at<Vec3b>(row, col)[0] * alphaReserve + BChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[1] = img.at<Vec3b>(row, col)[1] * alphaReserve + GChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[2] = img.at<Vec3b>(row, col)[2] * alphaReserve + RChannel * (1 - alphaReserve);
        }
    }
    for (int _col = 0; _col < right; _col++) {
        alphaReserve = _col / (double) right;
        int col = w - _col - 1;
        for (int row = 0; row < h; row++) {
            img.at<Vec3b>(row, col)[0] = img.at<Vec3b>(row, col)[0] * alphaReserve + BChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[1] = img.at<Vec3b>(row, col)[1] * alphaReserve + GChannel * (1 - alphaReserve);
            img.at<Vec3b>(row, col)[2] = img.at<Vec3b>(row, col)[2] * alphaReserve + RChannel * (1 - alphaReserve);
        }
    }

    outMat = img;
    return 0;
}

int opencv_tool::maskTranslucent(const Mat &inMat, Mat &outMat, cv::Scalar color) {
    return maskTranslucent(inMat, outMat, inMat.cols / 10, inMat.cols / 10, inMat.rows / 10, inMat.rows / 10, color);
}


#ifdef ENABLE_GTK3

GdkPixbuf *opencv_tool::MatToGdkPixbuf(const cv::Mat& inMat) {
    // IplImage -> GdkPixbuf
    GdkPixbuf *src = gdk_pixbuf_new_from_data(
            (const guchar *) inMat.data,
            GDK_COLORSPACE_RGB,
            false,
            8,
            inMat.size().width,
            inMat.size().height,
            inMat.step,
            nullptr,
            nullptr);
    return src;
}

#endif

#endif