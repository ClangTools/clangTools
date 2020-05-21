// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.


    This example is essentially just a version of the face_landmark_detection_ex.cpp
    example modified to use OpenCV's VideoCapture object to read from a camera instead
    of files.


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.
*/

#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <src/logger/logger.h>
#include <opencv_tool.h>

using namespace dlib;
using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
    try {
        logger::instance()->init_default();

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        // deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;
        deserialize("shape_predictor_5_face_landmarks.dat") >> pose_model;

        // Grab and process frames until the main window is closed by the user.
        // Grab a frame
        cv::Mat temp = imread(argc == 2 ? argv[1] : "headPose.jpg", IMREAD_ANYCOLOR);

        auto start = logger::get_time_tick();
        resize(temp, temp, cv::Size(320, 180));
        // opencv_tool::rotate180(temp);

        // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
        // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
        // long as temp is valid.  Also don't do anything to temp that would cause it
        // to reallocate the memory which stores the image as that will make cimg
        // contain dangling pointers.  This basically means you shouldn't modify temp
        // while using cimg.
        cv_image<bgr_pixel> cimg(temp);

        logger::instance()->i(__FILENAME__, __LINE__, "%lld", logger::get_time_tick() - start);
        // Detect faces
        std::vector<dlib::rectangle> faces = detector(cimg, 0);
        logger::instance()->d(__FILENAME__, __LINE__, "%lld : faces.size() : %lu", logger::get_time_tick() - start,
                              faces.size());
        // Find the pose of each face.
        std::vector<full_object_detection> shapes;
        for (unsigned long i = 0; i < faces.size(); ++i)
            shapes.push_back(pose_model(cimg, faces[i]));
        logger::instance()->d(__FILENAME__, __LINE__, "%lld", logger::get_time_tick() - start);

        // Display it all on the screen
    }
    catch (serialization_error &e) {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl << e.what() << endl;
    }
    catch (exception &e) {
        cout << e.what() << endl;
    }
}

