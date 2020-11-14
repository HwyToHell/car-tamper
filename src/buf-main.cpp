#include "../inc/motionbuffer.h"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <chrono>
#include <iostream>
#include <string>
#include <thread>


void fn_motion_detection(MotionBuffer& buf, cv::Mat& frameForDetection, bool& isMainThreadRunning) {
    std::cout << "thread motion detection started" << std::endl;

    while (isMainThreadRunning) {
        frameForDetection = buf.getFrameForDetection();
        std::cout << "frame received" << std::endl;
    }
}


int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;
    const cv::Scalar red			= cv::Scalar(0,0,255);
    const cv::Scalar blue			= cv::Scalar(255,0,0);
    const cv::Scalar black			= cv::Scalar(0,0,0);

    const cv::Mat blackFrame(240, 320, CV_8UC3, black);
    cv::Mat frame(240, 320, CV_8UC3, black);
    cv::Mat frameForDetection(240, 320, CV_8UC3, blue);
    int cntFrame = 0;

    MotionBuffer buf(50, 10);
    bool isMainAlive = true;
    std::thread thread_motion_detection( fn_motion_detection,
                                         std::ref(buf),
                                         std::ref(frameForDetection),
                                         std::ref(isMainAlive) );

    while (true) {
        ++cntFrame;

        blackFrame.copyTo(frame);
        cv::putText(frame, to_string(cntFrame), cv::Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 2, red, 3);
        cv::imshow("video", frame);
        buf.writeFrameToBuffer(frame);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cv::imshow("detection", frameForDetection);

        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            isMainAlive = false;
            buf.stopDetection();
            break;
        }
    }

    thread_motion_detection.join();

    return 0;

}
