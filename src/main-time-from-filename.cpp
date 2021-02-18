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
#include <experimental/filesystem>
#include <iostream>
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


    return 0;
}
