// project specific
#include "../inc/motionbuffer.h"
#include "../inc/backgroundsubtraction.h"
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



// test read opencv video capture
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    using namespace std;

    QString videoFile = QFileDialog::getOpenFileName(nullptr,
         "Select video file",
         QDir::currentPath(),
        "Video files (*.avi *.mp4)" );
    std::string file(videoFile.toUtf8());
    cout << file << endl;

    cv::VideoCapture cap(file);
    cv::Mat frame;
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
