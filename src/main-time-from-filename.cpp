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
#include <chrono>
#include <ctime>
#include <experimental/filesystem>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <sstream>
#include <string>



// test MotionBuffer saveToDisk by using video capture simulation
int main_time_from_filename(int argc, char *argv[]) {
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

    // convert tm to chrono::time_point
    time_t epochTime = mktime(&t);
    std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::from_time_t(epochTime);
    cout << "epoch time: " << epochTime << std::endl;

    auto startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count();
    cout << "time point milli sec: " << startTimeMs << std::endl;

    cout << "start time put_time: " << std::put_time(std::localtime(&epochTime), "%F %T") << endl;

    auto offset = std::chrono::milliseconds(1055);
    auto startTime1 = startTime + offset;
    auto epochMs1 = std::chrono::duration_cast<std::chrono::milliseconds>(startTime1.time_since_epoch()).count();
    cout << "time point milli sec: " << epochMs1 << std::endl;

    // double rep ändert den return type von count() nach double
    // dieser Typ erzeugt bei to_time_t einen Typfehler
    auto offsetFloat = std::chrono::duration<double>(0.011);
    std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> startTime2 = startTime + offsetFloat;
    auto epochMs2 = std::chrono::duration_cast<std::chrono::milliseconds>(startTime2.time_since_epoch()).count();
    cout << "time point milli sec float: " << epochMs2 << std::endl;


    return 0;
}
