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
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    using namespace std;

    QString paramFile = QFileDialog::getOpenFileName(nullptr,
         "Select json file",
         QDir::currentPath(),
        "Video files (*.json)" );
    std::string file(paramFile.toUtf8());
    cout << file << endl;

    cv::FileStorage fs;
    fs.open(file, cv::FileStorage::Mode::READ);
    if (!fs.isOpened()) {
        cout << "file storag not open"<< endl;
        return -1;
    }

    cv::FileNode node = fs.root();
    cout << "size:    " << node.size() << endl;
    cout << "empty:   " << node.empty() << endl;
    cout << "isMap:   " << node.isMap() << endl;
    cout << "isNamed: " << node.isNamed() << endl;
    cout << "isNone:  " << node.isNone() << endl;
    cout << "name:    " << node.name() << endl;

    for (cv::FileNodeIterator it = node.begin(); it != node.end(); ++it) {
        cout << "sub node:" << (*it).name() << endl;
    }

    //cout << node << endl;

    return 0;
}
