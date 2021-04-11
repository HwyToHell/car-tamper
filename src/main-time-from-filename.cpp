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



// determine recording time from file name
int main_time_from_file(int argc, char *argv[]) {
// int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    using namespace std;
    namespace fs = std::experimental::filesystem;

    /*
    QApplication a(argc, argv);
    QString videoFile = QFileDialog::getOpenFileName(nullptr,
         "Select video file",
         QDir::currentPath(),
        "Video files (*.avi *.mp4)" );
    std::string videoPathName(videoFile.toUtf8());
    //cout << videoPathName << endl;

    fs::path videoPath(videoPathName);
    // fs::path videoPath("/home/holger/app-dev/print-file-tree.txt");
    videoPath.replace_extension();
    cout << videoPath.filename() << endl;

    std::stringstream ss;
    // convert path to string to omit double quotes and avoid get_time parsing error
    ss << videoPath.filename().string();
    cout << ss.str() << endl;

    std::tm t{};
    time_t zeroDay = std::time(nullptr);
    t = *std::localtime(&zeroDay);

    ss.imbue(std::locale("de_DE.utf-8"));
    ss >> std::get_time(&t, "%Y-%m-%d_%Hh%Mm%Ss");
    cout << "dst: " << t.tm_isdst << endl;

    if (ss.fail()) {
        cout << "parse failed" << endl;
        if (ss.rdstate() == std::ios_base::badbit)
            cout << "badbit" << endl;
        if (ss.rdstate() == std::ios_base::eofbit)
            cout << "badbit" << endl;
        if (ss.rdstate() == std::ios_base::failbit)
            cout << "badbit" << endl;
    } else {
        cout << std::put_time(&t, "%c") << endl;
    }
    */

    std::stringstream ssWinter("2020-11-06_14h00m00s");
    std::stringstream ssSummer("2021-04-09_14h00m00s");
    time_t now = std::time(nullptr);
    std::tm tWinter{};
    std::tm tSummer{};
    //tWinter = *std::localtime(&now);
    //tSummer = *std::localtime(&now);

    ssWinter >> std::get_time(&tWinter, "%Y-%m-%d_%Hh%Mm%Ss");
    ssSummer >> std::get_time(&tSummer, "%Y-%m-%d_%Hh%Mm%Ss");
    tWinter.tm_isdst = -1;
    tSummer.tm_isdst = -1;

    time_t timeWinter = std::mktime(&tWinter);
    time_t timeSummer = std::mktime(&tSummer);
    cout << "winter put_time: " << std::put_time(std::localtime(&timeWinter), "%F %T") << endl;
    cout << "summer put_time: " << std::put_time(std::localtime(&timeSummer), "%F %T") << endl;

    /*
    // check, if mktime sets correct time zone, if isdst = -1
    t.tm_isdst = -1;

    // convert tm to chrono::time_point
    time_t epochTime = mktime(&t);
    std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::from_time_t(epochTime);
    cout << "epoch time: " << epochTime << std::endl;

    auto startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count();
    //cout << "time point milli sec: " << startTimeMs << std::endl;
    std::tm* pStartTm;
    pStartTm = std::localtime(&epochTime);
    cout << "start time put_time: " << std::put_time(std::localtime(&epochTime), "%F %T") << endl;
    cout << "dst: " << pStartTm->tm_isdst << endl;

    auto offset = std::chrono::milliseconds(10000);
    auto endTime = startTime + offset;
    auto endTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime.time_since_epoch()).count();
    time_t endTimeT = std::chrono::system_clock::to_time_t(endTime);
    cout << "start + offset put_time: " << std::put_time(std::localtime(&endTimeT), "%F %T") << endl;
    */


    return 0;
}
