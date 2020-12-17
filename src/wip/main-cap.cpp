#include "../car-tamper-test/inc/video-capture-simu.h"
#include "../inc/motionbuffer.h"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <chrono>
#include <iostream>
#include <string>



// test video capture simulation
int main_cap(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;

    VideoCaptureSimu cap(5);
    cv::Mat frame;

    cap.set( VidCapProps::fps, "20");
    cout << "fps:        " << cap.get(VidCapProps::fps) << endl;
    cap.set( VidCapProps::frameSize, "320x240");
    cout << "frame size: " << cap.get(VidCapProps::frameSize) << endl;
    cap.set( VidCapProps::isLogging, "0");
    cout << "logging:    " << cap.get(VidCapProps::isLogging) << endl;
    return 0;

    // std::this_thread::sleep_for(std::chrono::milliseconds(200));
    while (true) {

        cap.read(frame);
        cout << getTimeStamp(TimeResolution::ms) << " main received " << cap.getFrameCount() << endl;
        cv::imshow("video", frame);


        if (cv::waitKey(10) == 27) {
            std::cout << "esc -> end video processing" << std::endl;
            break;
        }
    }
    return 0;
}
