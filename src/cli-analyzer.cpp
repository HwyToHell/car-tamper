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
#include <thread>

enum class Error
{
    FileExist   = -10,
    ParseTime   =  -9,
    FileOpen    =  -8,
    AllFrames   =  -7,
    OK          =   0
};


const std::map<Error, std::string> errorMsg
{
    {Error::FileExist, "file does not exist"},
    {Error::ParseTime, "filename does not match time convention: YYYY-MM-DD_HHhMMmSSs"},
    {Error::FileOpen, "cannot open video file"},
    {Error::AllFrames, "was not able to read all frames from video file"}
};

struct ParamMotionBuffer
{
    ParamMotionBuffer() {}
    double      fps;
    size_t      pre;
    size_t      post;
    std::string videoDir;
};


struct ParamMotionDetector
{
    ParamMotionDetector() {}
    double      bgrSubThreshold;
    int         minMotionDuration;
    int         minMotionIntensity;
    cv::Rect    roi;
};


struct Params
{
    Params() {}
    ParamMotionBuffer   buffer;
    ParamMotionDetector detector;
};


Error analyzeMotion(Params params, std::string fileName)
{
    // file exists
    namespace fs = std::experimental::filesystem;
    fs::path videoPath(fileName);
    if (!fs::exists(videoPath))
        return Error::FileExist;

    // parse time from file name
    std::stringstream ss(videoPath.filename());
    std::tm startTime;
    ss >> std::get_time(&startTime, "%Y-%m-%d_%Hh%Mm%Ss");
    if (ss.fail())
        return Error::ParseTime;

    // output dir
    std::stringstream outDirectory;
    outDirectory << std::put_time(&startTime, "%F_%Hh%Mm%Ss");

    // get fps and frame count of video file
    cv::VideoCapture cap;
    cap.open(fileName);
    if (!cap.isOpened())
        return Error::FileOpen;
    double fps = cap.get(cv::CAP_PROP_FPS);
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    // analyze file
    MotionBuffer buffer(params.buffer.pre,
                        fps,
                        outDirectory.str(),
                        "log",
                        false,
                        true);
    buffer.postBuffer(params.buffer.post);
    buffer.startTime(startTime);

    MotionDetector detector;
    detector.bgrSubThreshold(params.detector.bgrSubThreshold);
    detector.minMotionDuration(params.detector.minMotionDuration);
    detector.minMotionIntensity(params.detector.minMotionIntensity);
    detector.roi(params.detector.roi);

    std::cout << fileName << std::endl;

    int frameCount = 0;
    cv::Mat frame;
    while (cap.read(frame)) {
        ++frameCount;
        buffer.pushToBuffer(frame);
        detector.motionDuration();
        bool isMotion = detector.isContinuousMotion(frame);
        if (isMotion) {
            if (!buffer.isSaveToDiskRunning()) {
                buffer.setSaveToDisk(true);
            }
        } else {
            buffer.setSaveToDisk(false);
        }
        // progress in per cent
        if (frameCount % 30 == 0) {
            std::cout << (frameCount * 100 / totalFrames) << "%\r";
            std::cout.flush();
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    buffer.releaseBuffer();
    cap.release();

    if (frameCount == totalFrames)
        return Error::OK;
    else
        return Error::AllFrames;
}



// TODO: params mit passenden Werten initialisieren
int main(int argc, char *argv[])  {

    Params params;
    params.buffer.fps = 30;
    params.buffer.pre = 30;
    params.buffer.post = 30;
    params.buffer.videoDir = "videos";
    params.detector.roi = cv::Rect(0,0,0,0);
    params.detector.bgrSubThreshold = 50;
    params.detector.minMotionDuration = 10;
    params.detector.minMotionIntensity = 10;

    if (argc < 2) {
        std::cout << "usage: tamper filename" << std::endl;
        return -1;
    }

    Error error = analyzeMotion(params, argv[1]);
    std::cout << std::endl;

    if (error == Error::OK)
        std::cout << "successful" << std::endl;
    else {
        std::cout << errorMsg.find(error)->second << std::endl;
    }
    return 0;
}
