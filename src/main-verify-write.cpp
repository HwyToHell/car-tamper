// project specific
#include "../inc/backgroundsubtraction.h"
#include "../inc/motionbuffer.h"
#include "../inc/time-stamp.h"
#include "../car-tamper-test/inc/video-capture-simu.h"

// opencv
#include <opencv2/opencv.hpp>

// qt
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QString>

// std
#include <iostream>
#include <string>



// test MotionBuffer saveToDisk
int main_verify_write(int argc, char *argv[]) {
    (void)argc; (void)argv;
    using namespace std;

    const size_t fps = 30;
    VideoCaptureSimu cap(fps, "1920x1080");
    double fpsSource = cap.get(cv::CAP_PROP_FPS);
    cout << "fps generated by source: " << fpsSource << endl;


    const size_t ringBufferSize = 1;
    MotionBuffer buf(ringBufferSize, fpsSource, "video", "test2");

    cv::Mat frame;
    int cnt = 0;

    while (cap.read(frame)) {
        ++cnt;
        buf.pushToBuffer(frame);

        if (cnt > 60) {
            buf.setSaveToDisk(true);
        }

        if (cnt > 70) {
            buf.setSaveToDisk(false);
        }

        if (cnt > 72) break;

        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }
    }
    cap.release();
    return 0;
}
