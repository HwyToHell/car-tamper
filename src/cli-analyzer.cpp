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
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
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
    Params();
    ~Params();
    ParamMotionBuffer   buffer;
    ParamMotionDetector detector;
    void                loadSettings();
    void                saveSettings();
};


Params::Params()
{
    loadSettings();
}


Params::~Params()
{
    std::cout << "d'tor save settings" << std::endl;
    saveSettings();
}


void Params::loadSettings()
{
    QSettings settings;

    settings.beginGroup("MotionBuffer");
    buffer.fps = settings.value("fps", 30).toDouble();
    buffer.pre = static_cast<size_t>(settings.value("preBuffer", 30).toInt());
    buffer.post = static_cast<size_t>(settings.value("postBuffer", 10).toInt());
    buffer.videoDir = settings.value("videoDir", "videos").toString().toStdString();
    settings.endGroup();

    settings.beginGroup("MotionDetector");
    detector.bgrSubThreshold = settings.value("bgrSubThreshold", 50).toDouble();
    detector.minMotionDuration = settings.value("minMotionDuration", 10).toInt();
    detector.minMotionIntensity = settings.value("minMotionIntensity", 10).toInt();
    QRect qRoi = settings.value("roi", QRect(0,0,0,0)).toRect();
    detector.roi = cv::Rect(qRoi.x(), qRoi.y(), qRoi.width(),qRoi.height());
    settings.endGroup();
}

void Params::saveSettings()
{
    QSettings settings;

    settings.beginGroup("MotionBuffer");
    settings.setValue("fps", buffer.fps);
    settings.setValue("preBuffer", static_cast<int>(buffer.pre));
    settings.setValue("postBuffer", static_cast<int>(buffer.post));
    settings.setValue("videoDir", QString::fromStdString(buffer.videoDir));
    settings.endGroup();

    settings.beginGroup("MotionDetector");
    settings.setValue("bgrSubThreshold", detector.bgrSubThreshold);
    settings.setValue("minMotionDuration", detector.minMotionDuration);
    settings.setValue("minMotionIntensity", detector.minMotionIntensity);
    QRect qRoi(detector.roi.x, detector.roi.y, detector.roi.width, detector.roi.height);
    settings.setValue("roi", qRoi);
    settings.endGroup();
}


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
    QApplication a(argc, argv);
    QApplication::setOrganizationName("grzonka");
    Params params;
    std::cout << "roi: " << params.detector.roi << std::endl;

    /*
    qDebug() << "appName: " << a.applicationName();
    qDebug() << "orgName: " << a.organizationName();
    qDebug() << "config: " << QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    qDebug() << "file: " << settings.fileName();
    */

    //a.applicationName();
    return 0;


    QString videoFile = QFileDialog::getOpenFileName(nullptr,
         "Select video file",
         QDir::currentPath(),
        "Video files (*.avi *.mp4)" );
    std::string videoPathName(videoFile.toUtf8());
    std::cout << videoPathName << std::endl;



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
