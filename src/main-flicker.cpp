// project specific
#include "../inc/motion-detector.h"
#include "../inc/motionbuffer.h"

// opencv
#include <opencv2/opencv.hpp>
#include <opencv2/core/fast_math.hpp>

// qt
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QString>

// std
#include <chrono>
#include <iostream>
#include <string>

cv::Mat hist256(cv::Mat& imageIn)
{
    // histogram
    int bins = 256;
    int channels[] = {0};
    float range[] = {0, 256};
    const float* ranges = {range};

    cv::Mat hist;
    cv::calcHist(&imageIn, 1, channels, cv::Mat(), hist, 1, &bins, &ranges);
    double maxVal = 0;
    //cout << "size of histogram: " << hist.size() << endl;

    cv::minMaxLoc(hist, nullptr, &maxVal);
    int histHeight = static_cast<int>(range[1]);

    cv::Vec3b black(0, 0, 0);
    cv::Vec3b white(255, 255, 255);
    cv::Mat3b histImageOut(histHeight, bins, black);
    for(int b = 0; b < bins; ++b) {
        const double binVal = static_cast<double>(hist.at<float>(b));
        const int height = cvRound(binVal*histHeight/maxVal);
        cv::line(histImageOut, cv::Point(b, histHeight-height), cv::Point(b, histHeight), white);
    }
    return histImageOut;
}

// use opencv filter functions to reduce camera flicker
int main_flicker(int argc, char *argv[])
{
    QApplication a(argc, argv);
    using namespace std;

    QString videoFile = QFileDialog::getOpenFileName(nullptr,
         "Select video file",
         QDir::currentPath(),
        "Video files (*.avi *.mp4 *.mkv)" );
    std::string file(videoFile.toUtf8());

    cv::VideoCapture cap(file, cv::CAP_FFMPEG);
    if (!cap.isOpened()) {
        cout << "cannot open: " << file << endl;
        return -1;
    }

    MotionDetector mdet;
    cv::Mat frame;
    int cnt = 0;
    while (cap.read(frame)) {

        //cv::imshow("video", frame);

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::imshow("gray", gray);

        mdet.hasFrameMotion(frame);
        //cv::imshow("motion", mdet.getMotionFrame());

        // histogram
        cv::Mat histImage = hist256(gray);
        cv::imshow("hist", histImage);

        cv::Mat eqGray;
        cv::equalizeHist(gray, eqGray);
        cv::Mat histEq = hist256(eqGray);
        cv::imshow("eq gray", eqGray);
        cv::imshow("eq hist", histEq);


        cout << "frame: " << ++cnt << "\r";
        cout.flush();

        if (cv::waitKey(0) == 27) {
            std::cout << "esc -> end video processing" << std::endl;
            break;
        }
    }
    cap.release();
    return 0;
}
