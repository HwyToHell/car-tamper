// project specific
#include "../inc/backgroundsubtraction.h"
#include "../inc/motion-detector.h"
#include "../inc/motionbuffer.h"
#include "../inc/time-stamp.h"
#include "../inc/video-capture-simu.h"

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



// test MotionBuffer saveToDisk by analyzing video file
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    using namespace std;


    const size_t sourceFps = 30;
    VideoCaptureSimu cap(sourceFps, "640x480");
    double readSourceFps = cap.get(cv::CAP_PROP_FPS);
    cout << "fps generated by source: " << readSourceFps << endl;


    const size_t bufSize = 30;
    MotionBuffer mb(bufSize, readSourceFps, "video", "log");

    MotionDetector mdet;
    cv::Mat frame;

    //cap.setMode(30, 50);

    int cnt = 0;
    while (cap.read(frame)) {
        ++cnt;


        /*
         * test grey scale
        if (cnt % static_cast<int>(sourceFps) == 0) {
            int greyScale = cnt / static_cast<int>(sourceFps) % 10 * 10;
            cout << "grey scale: " << greyScale << endl;
            cap.setMode(30, greyScale, true);
        }
        */


        mb.pushToBuffer(frame);
        bool isMotion = mdet.hasFrameMotion(frame);
        int duration = mdet.updateMotionDuration(isMotion);
        bool isSaveToDisk = mdet.enableSaveToDisk(mb);
        cout << "----------------------------------------------------------------------" << endl;
        cout << "pass: " << cnt << " | frame has motion: " << isMotion;
        cout <<  " | motion duration: " << duration << " | save to disk: " << isSaveToDisk << endl;
        cout << "----------------------------------------------------------------------" << endl;

        //   0: write frames w/o motion
        //  30: enable vid-cap motion ->     40: saveToDisk activated
        //  70: disable vid-cap motion ->    80: saveToDisk de-activated
        // 100: stop vid-cap
        if (cnt == 40) {
            cap.setMode(GenMode::motionAreaAndTime, 10, 50);
        }
        if (cnt == 80) {
            // TODO vid cap -> setTimeStampMode
            // or setMode timeStamp, motionArea, motionAreaAndTime
            cap.setMode(GenMode::timeStamp);
        }
        if (cnt == 100)
            break;


        cv::imshow("video", frame);

        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }
    }

    cap.release();
    cout << endl;
    cout << "finished" << endl;
    return 0;
}
