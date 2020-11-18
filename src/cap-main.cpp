#include "../car-tamper-test/inc/video-capture-simu.h"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <chrono>
#include <iostream>
#include <string>



// test video capture simulation
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;

    VideoCaptureSimu cap(5);

    cv::Mat frame;


    while (true) {
        cap.read(frame);
        cv::imshow("video", frame);
        cout << "main" << endl;

        if (cv::waitKey(10) == 27) {
            std::cout << "esc -> end video processing" << std::endl;
            break;
        }
    }
    return 0;
}
