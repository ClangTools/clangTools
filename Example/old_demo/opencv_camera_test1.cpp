//
// Created by caesar kekxv on 2020/3/12.
// http://scientistengineer.blogspot.com/2015/09/trying-to-generate-hdr-picture-from.html
//


#include "opencv2/opencv.hpp"
#include <vector>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

int main(int argc, char ** argv)
{
    setenv("DISPLAY", "localhost:10.0", 1);
    vector< Mat> images;
    vector< float> times;


    //float times[5];
    VideoCapture camera;
    camera.open(0);

//    camera.set(CAP_PROP_FRAME_WIDTH,640);
//    camera.set(CAP_PROP_FRAME_HEIGHT,480);

    //for(;;)
    //{
    Mat defaultPic[5];
    camera.set(CAP_PROP_EXPOSURE,-1);
    //times[0] = 1;

    camera >> defaultPic[0];
    times.push_back(1);
    images.push_back(defaultPic[0]);
    namedWindow( "picture -1", WINDOW_AUTOSIZE );
    imshow("picture -1", defaultPic[0]);

    camera.set(CAP_PROP_EXPOSURE,2);
    //times[1] = 0.2;
    camera >> defaultPic[1];
    times.push_back(2);
    images.push_back(defaultPic[1]);
    namedWindow( "picture -5", WINDOW_AUTOSIZE );
    imshow("picture -5", defaultPic[1]);

    camera.set(CAP_PROP_EXPOSURE,3);
    //times[2] = 0.143;
    camera >> defaultPic[2];
    times.push_back(3);
    images.push_back(defaultPic[2]);
    namedWindow( "picture -7", WINDOW_AUTOSIZE );
    imshow("picture -7", defaultPic[2]);

    camera.set(CAP_PROP_EXPOSURE,4);
    //times[3] = 0.11;
    camera >> defaultPic[3];
    times.push_back(4);
    images.push_back(defaultPic[3]);
    namedWindow( "picture -9", WINDOW_AUTOSIZE );
    imshow("picture -9", defaultPic[3]);

    camera.set(CAP_PROP_EXPOSURE,5);
    //times[4] = 0.083;
    camera >> defaultPic[4];
    times.push_back(5);
    images.push_back(defaultPic[4]);
    imshow("img12.png",defaultPic[4]);

    camera.set(CAP_PROP_EXPOSURE,7);
    //times[4] = 0.083;
    camera >> defaultPic[4];
    times.push_back(7);
    images.push_back(defaultPic[4]);
    imshow("img13.png",defaultPic[4]);

    camera.set(CAP_PROP_EXPOSURE,9);
    //times[4] = 0.083;
    camera >> defaultPic[4];
    times.push_back(9);
    images.push_back(defaultPic[4]);
    namedWindow( "picture -12", WINDOW_AUTOSIZE );
    imshow("picture -12", defaultPic[4]);

    Mat response;
    Ptr< CalibrateDebevec> calibrate = createCalibrateDebevec();
    calibrate->process(images, response, times);

    Mat hdr;
    Ptr< MergeDebevec > merge_debevec = createMergeDebevec();
    merge_debevec->process(images, hdr, times, response);

    Mat ldr;
    auto tonemap = cv::createTonemapDrago(2.2f);
    tonemap->process(hdr, ldr);

    Mat fusion;
    // createTonemapDurand
    auto merge_mertens = createMergeMertens();
    merge_mertens->process(images, fusion);

    imshow("fusion.png", fusion * 255);
    imshow("ldr.png", ldr * 255);
    imshow("hdr.hdr", hdr);

    waitKey(0);
//}
    return 0;
}