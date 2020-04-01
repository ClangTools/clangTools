/*-----------------------------------------------------------------------------
/
/
/----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv_tool.h>
/*---------------------------------------------------------------------------*/
using namespace std;
using namespace cv;

/*---------------------------------------------------------------------------*/
uint8_t saturated_add(uint8_t val1, int8_t val2);

/*---------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    Mat rawImg;
    Mat dithImg;

    if (argc != 2) {
        printf("> Usage: ./dithering [path_to_image]\n");
        return -1;
    }

    /* Load the image and convert to grayscale first */
    rawImg = imread(argv[1]);
    cvtColor(rawImg, dithImg, COLOR_BGR2GRAY);

    std::vector<std::vector<unsigned char>> outputPtr;
    opencv_tool::ImgDithering(dithImg, outputPtr);

    /* Show results. */
    imshow("Raw Image", rawImg);
    imshow("Dithered Image", dithImg);
    printf("> Press any key to exit ...\n");

    waitKey(0);
    return 0;
}
