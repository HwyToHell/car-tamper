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


    time_t time = 0;
    cout << put_time(localtime(&time), "%c") << endl;
    return 0;

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

    // set time
    t.tm_hour = t.tm_min = t.tm_sec = 0;
    cout << "time changed: " << std::put_time(&t, "%Y-%m-%d_%Hh%Mm%Ss") << endl;


    // simulate reading from file
    const size_t bufSize = 40;
    const size_t fpsOut = 30;
    VideoCaptureSimu vcs(InputMode::videoFile, "640x480", fpsOut);

    // automatically setting roi
    cv::Mat frame;
    vcs.read(frame);
    MotionDetector mdet;
    mdet.isContinuousMotion(frame);
    cout << "roi: " << mdet.roi() << endl;

    // motion buffer with time from file
    MotionBuffer mb(bufSize, fps, "video", "log", false, true);
    mb.startTime(t);
    time_t reReadTime = mb.startTime();
    cout << "re-read time_t: " << reReadTime << endl;
    TimePoint reTp = std::chrono::system_clock::from_time_t(reReadTime);
    cout << "re-read start time: " << getTimeStamp(TimeResolution::ms, reTp) << endl;



    int cnt = 0;
    bool isSaveToDisk = false;
    while (vcs.read(frame)) {
        ++cnt;

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



        /*
        cout << "----------------------------------------------------------------------------" << endl;
        cout << "pass: " << cnt << " motion: " << isMotion <<  " | motion duration: ";
        cout << duration << " | setS2D: " << isSaveToDisk << " | isS2DRunning: " << mb.isSaveToDiskRunning() << endl;
        cout << "-----------------------------------------------------------------------------" << endl;
        */

        //   0: write frames w/o motion
        //  30: enable vid-cap motion ->     40: saveToDisk activated
        //  70: disable vid-cap motion ->    80: saveToDisk de-activated
        // 100: stop vid-cap
        if (cnt == 100) {
            vcs.setMode(GenMode::motionAreaAndTime, 10, 50);
        }
        if (cnt == 120) {
            // TODO vid cap -> startTimeStampMode
            // or setMode timeStamp, motionArea, motionAreaAndTime
            vcs.setMode(GenMode::timeStamp);
        }
        if (cnt == 170)
            break;

        /*
        cv::imshow("video", frame);

        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }
        */
    }

    this_thread::sleep_for(100ms);
    vcs.release();
    cout << endl;
    cout << "finished" << endl;
    return 0;
}
