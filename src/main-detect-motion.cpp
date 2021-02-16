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



// test MotionBuffer saveToDisk by using video capture simulation
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    using namespace std;

    /* test cv::rectangle
    const cv::Vec3b black  = cv::Vec3b(0,0,0);
    const cv::Size frameSize(640,480);
    cv::Mat frame(frameSize, CV_8UC3, black);
    cv::Rect motionRect(0,0,100,frameSize.width);
    cv::Vec3b greyShade = cv::Vec3b(0, 128, 128);
    cv::rectangle(frame, motionRect, greyShade, cv::FILLED, cv::LINE_4);
    cv::imshow("video", frame);
    cv::waitKey(0);
    return 0;
    */

    const size_t bufSize = 10;
    const size_t fps = 10;
    VideoCaptureSimu vcs(InputMode::camera, "640x480", fps);
    MotionBuffer mb(bufSize, fps, "video", "log");

    MotionDetector mdet;
    cv::Mat frame;
    vcs.read(frame);
    cv::Size frameSize = frame.size();
    cv::Rect roi(cv::Point(0,0), frameSize);
    cout << "roi: " << roi << endl;
    mdet.roi(roi);

    //cap.setMode(30, 50);

    int cnt = 0;
    bool isSaveToDisk = false;
    while (vcs.read(frame)) {
        ++cnt;


        /*
         * test grey scale
        if (cnt % static_cast<int>(fps) == 0) {
            int greyScale = cnt / static_cast<int>(fps) % 10 * 10;
            cout << "grey scale: " << greyScale << endl;
            vcs.setMode(30, greyScale, true);
        }
        */


        mb.pushToBuffer(frame);
        int duration = mdet.motionDuration();
        bool isMotion = mdet.isContinuousMotion(frame);


        if (isMotion) {
            if (!mb.isSaveToDiskRunning()) {
                mb.setSaveToDisk(true);
                isSaveToDisk = true;
            }
        } else {
            mb.setSaveToDisk(false);
            isSaveToDisk = false;
        }



        cout << "----------------------------------------------------------------------------" << endl;
        cout << "pass: " << cnt << " motion: " << isMotion <<  " | motion duration: ";
        cout << duration << " | setS2D: " << isSaveToDisk << " | isS2DRunning: " << mb.isSaveToDiskRunning() << endl;
        cout << "-----------------------------------------------------------------------------" << endl;

        //   0: write frames w/o motion
        //  30: enable vid-cap motion ->     40: saveToDisk activated
        //  70: disable vid-cap motion ->    80: saveToDisk de-activated
        // 100: stop vid-cap
        if (cnt == 20) {
            vcs.setMode(GenMode::motionAreaAndTime, 10, 50);
        }
        if (cnt == 40) {
            // TODO vid cap -> setTimeStampMode
            // or setMode timeStamp, motionArea, motionAreaAndTime
            vcs.setMode(GenMode::timeStamp);
        }
        if (cnt == 80)
            break;


        cv::imshow("video", frame);

        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }
    }

    vcs.release();
    cout << endl;
    cout << "finished" << endl;
    return 0;
}
