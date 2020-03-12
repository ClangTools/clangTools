//
// Created by caesar kekxv on 2020/3/12.
// http://www.pauldebevec.com/Research/HDR/
// https://docs.opencv.org/master/d6/df5/group__photo__hdr.html
//

#include "opencv2/opencv.hpp"
#include <vector>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

void loadExposureSeq(String, vector<Mat> &, vector<float> &);

int main(int argc, char **argv) {
    CommandLineParser parser(argc, argv, "{@input | | Input directory that contains images and exposure times. }");
    vector<Mat> images;
    vector<float> times;
    loadExposureSeq(parser.get<String>("@input"), images, times);
    Mat response;
    Ptr<CalibrateDebevec> calibrate = createCalibrateDebevec();
    calibrate->process(images, response, times);
    Mat hdr;
    Ptr<MergeDebevec> merge_debevec = createMergeDebevec();
    merge_debevec->process(images, hdr, times, response);
    Mat ldr;
    Ptr<Tonemap> tonemap = createTonemap(2.2f);
    tonemap->process(hdr, ldr);
    Mat fusion;
    Ptr<MergeMertens> merge_mertens = createMergeMertens();
    merge_mertens->process(images, fusion);
    imshow("fusion.png", fusion * 255);
    imshow("ldr.png", ldr * 255);
    imshow("hdr.hdr", hdr);
    waitKey();
    return 0;
}

void loadExposureSeq(String path, vector<Mat> &images, vector<float> &times) {
    path = path + "/";
    ifstream list_file((path + "list.txt").c_str());
    string name;
    float val;
    while (list_file >> name >> val) {
        Mat img = imread(path + name);
        images.push_back(img);
        times.push_back(1 / val);
    }
    list_file.close();
}