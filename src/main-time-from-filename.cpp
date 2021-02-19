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
#include <ctime>
#include <experimental/filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>



// test MotionBuffer saveToDisk by using video capture simulation
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    using namespace std;
    namespace fs = std::experimental::filesystem;

    QApplication a(argc, argv);
    QString videoFile = QFileDialog::getOpenFileName(nullptr,
         "Select video file",
         QDir::currentPath(),
        "Video files (*.avi *.mp4)" );
    std::string videoPathName(videoFile.toUtf8());
    cout << videoPathName << endl;

    fs::path videoPath(videoPathName);
    // fs::path videoPath("/home/holger/app-dev/print-file-tree.txt");
    cout << videoPath.filename() << endl;
    std::stringstream ss(videoPath.filename());

    std::tm t;
    ss >> std::get_time(&t, "%Y-%m-%d_%Hh%Mm%Ss");

    if (ss.fail()) {
        cout << "parse failed" << endl;
    } else {
        cout << std::put_time(&t, "%c") << endl;
    }

    cv::VideoCapture cap(videoPathName);
    if (!cap.isOpened()) {
        cout << "cannot open video file" << endl;
        return -1;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    double frames = cap.get(cv::CAP_PROP_FRAME_COUNT);
    double min = frames / fps / 60;

    cout << "fps:    " << fps << "\n"
         << "frames: " << frames << "\n"
         << "min:    " << min << endl;

    videoPath.remove_filename();
    for (auto entry : fs::directory_iterator(videoPath)) {
        cout << entry.path() << endl;
    }


    return 0;
}
