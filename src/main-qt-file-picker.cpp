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

#include <opencv2/core/utils/filesystem.hpp>



// test read opencv video capture
int main_qt_file(int argc, char *argv[])
{
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    QApplication a(argc, argv);
    using namespace std;


    // TODO delete
    cout << "cwd  " << cv::utils::fs::getcwd() << endl;
    cout << "time " << getTimeStamp(TimeResolution::ms_NoBlank) << endl;

    cv::Mat frame(cv::Size(320,240), CV_8UC3, cv::Vec3b(0,0,0));
    LogFrame logFrameFile("test");
    logFrameFile.create();
    logFrameFile.write(frame);
    cout << "ms elapsed: " << elapsedMs(start) << endl;

    std::this_thread::sleep_for(100ms);
    frame.at<int>(0,0) = 2;
    frame.at<int>(0,frame.rows-1) = 5;
    logFrameFile.write(frame);
    logFrameFile.close();
    cout << "cols: " << frame.cols << "  rows: " << frame.rows << endl;
    cout << frame.size() << endl;
    cout << "ms elapsed: " << elapsedMs(start) << endl;

    return 0;

    QString videoFile = QFileDialog::getOpenFileName(nullptr,
         "Select video file",
         QDir::currentPath(),
        "Video files (*.avi *.mp4)" );
    std::string file(videoFile.toUtf8());
    cout << file << endl;

    cv::VideoCapture cap(file);
    //cv::Mat frame;
    int cnt = 0;
    while (cap.read(frame)) {
        cout << "pass #" << ++cnt <<" - cnt from frame: " << frame.at<int>(0) << endl;
        //cv::imshow("video", frame);
        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }
    }
    cap.release();
    return 0;
}
