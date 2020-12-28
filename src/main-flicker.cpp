// project specific
#include "../inc/motion-detector.h"
#include "../inc/motionbuffer.h"

// opencv
#include <opencv2/opencv.hpp>

// qt
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QString>

// std
#include <chrono>
#include <iostream>
#include <string>



// use opencv filter functions to reduce camera flicker
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    using namespace std;

    QString videoFile = QFileDialog::getOpenFileName(nullptr,
         "Select video file",
         QDir::currentPath(),
        "Video files (*.avi *.mp4)" );
    std::string file(videoFile.toUtf8());

    cv::VideoCapture cap(file, cv::CAP_FFMPEG);
    if (!cap.isOpened()) {
        cout << "cannot open: " << file << endl;
        return -1;
    }

    MotionDetector mdet;
    cv::Mat frame;
    while (cap.read(frame)) {

        mdet.hasFrameMotion(frame);
        cv::imshow("video", frame);
        cv::imshow("motion", mdet.getMotionFrame());

        if (cv::waitKey(10) == 27) {
            std::cout << "esc -> end video processing" << std::endl;
            break;
        }
    }
    cap.release();
    return 0;
}
