// project specific
#include "../inc/backgroundsubtraction.h"
#include "../inc/motion-detector.h"
#include "../inc/motionbuffer.h"
#include "../inc/time-stamp.h"
#include "../inc/video-capture-simu.h"

// color cursor
#include "../cpp/inc/rlutil.h"

// opencv
#include <opencv2/opencv.hpp>

// qt
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QString>

// std
#include <chrono>
#include <cstdlib> // getenv
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
    //std::cout << "d'tor save settings" << std::endl;
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


void printProgress(std::string file, int progress)
{
    std::cout << file << "  " << progress << "%\r";
    std::cout.flush();
}


Error analyzeMotion(Params params, std::string fileName)
{
    // file exists
    namespace fs = std::experimental::filesystem;
    fs::path videoPath(fileName);
    if (!fs::exists(videoPath))
        return Error::FileExist;

    // parse time from file name
    std::stringstream ss;
    ss << videoPath.filename();
    time_t zeroDay = std::time(nullptr);
    // setting time zone
    std::tm startTime = *std::localtime(&zeroDay);
    ss >> std::get_time(&startTime, "%Y-%m-%d_%Hh%Mm%Ss");
    if (ss.fail())
        return Error::ParseTime;
    // always use winter time
    startTime.tm_isdst = 0;

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

    int frameCount = 0;
    int progress = 0;
    cv::Mat frame;
    rlutil::CursorHider hide;
    while (cap.read(frame)) {
        ++frameCount;
        buffer.pushToBuffer(frame);
        bool isMotion = detector.isContinuousMotion(frame);
        if (isMotion) {
            if (!buffer.isSaveToDiskRunning()) {
                buffer.setSaveToDisk(true);
            }
        } else {
            buffer.setSaveToDisk(false);
        }
        // progress in per cent
        if (frameCount % 10 == 0) {
            progress = (frameCount * 100 / totalFrames);
            printProgress(fileName, progress);
        }
    }
    // allow 1 second to release video writer, otherwise ffmpeg encoding errors are likely to occur
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    buffer.releaseBuffer();
    cap.release();

    if (frameCount == totalFrames) {
        std::cout << fileName << "  ";
        rlutil::setColor(10);
        std::cout << "OK   " << std::endl;
        rlutil::resetColor();
        return Error::OK;
    } else {
        std::cout << fileName << "  ";
        rlutil::setColor(12);
        std::cout << progress << "%" << std::endl;
        rlutil::resetColor();
        return Error::AllFrames;
    }
}


std::vector<std::string> getVideoFiles(QString path)
{
    std::vector<std::string> videoFiles;
    QDirIterator it(path, {"*.mp4", "*.wav"}, QDir::Files);
    while (it.hasNext()) {
        it.next();
        std::string file = it.filePath().toStdString();
        videoFiles.push_back(file);
    }
    return videoFiles;
}


std::vector<std::string> getVideoFilesFromArgs(QStringList args)
{
    std::vector<std::string> videoFiles;
    for(auto arg: args) {
        if (arg.endsWith(".avi") || arg.endsWith(".mp4")) {
            videoFiles.push_back(arg.toStdString());
        }
    }
    return videoFiles;
}


bool showRoi(std::string videoFile, cv::Rect roi)
{
    cv::VideoCapture cap;
    cap.open(videoFile);
    if (!cap.isOpened())
        return false;
    cv::Mat frame;
    if (!cap.read(frame))
        return false;

    std::cout << "frame size: " << frame.size() << ", roi: " << roi << std::endl;
    cv::rectangle(frame, roi, cv::Scalar(0,0,255), 2);
    cv::imshow("region for motion processing", frame);
    std::cout << "press any key to close preview" << std::endl;
    cv::waitKey(5000);
    cv::destroyWindow("region for motion processing");
    return true;
}


bool waitForEnter()
{
    using namespace std;
    cout << endl << "Press <enter> to exit" << endl;
    string str;
    getline(cin, str);
    return true;
}


// TODO: progress bar
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("grzonka");
    Params params;

    // parse command line args
    QCommandLineParser cmdLine;
    cmdLine.setApplicationDescription("Extract motion sequences of videoFile to separate files");
    cmdLine.addHelpOption();
    QCommandLineOption roiOption(QStringList() << "r" << "roi", "show roi before processing files");
    cmdLine.addOption(roiOption);
    cmdLine.addPositionalArgument("videoFile", "Process single video file");
    cmdLine.addPositionalArgument(".", "Process all video files in current directory");
    cmdLine.addPositionalArgument("", "Select directory with video files to process");

    cmdLine.process(a);
    const QStringList posArgs = cmdLine.positionalArguments();

    std::vector<std::string> videoFiles;
    // take video files from command line
    if (posArgs.size() > 0) {
        // take all video files of current directory
        if (posArgs.at(0) == ".") {
            QString curPath = QDir::currentPath();
            videoFiles = getVideoFiles(curPath);

        // take each argument as video input file
        } else {
            videoFiles = getVideoFilesFromArgs(posArgs);
        }

    // select directory and extract all video files
    } else {
        QString videoPath = QFileDialog::getExistingDirectory(nullptr,
                        "Select Directory",
                        QDir::currentPath(),
                        QFileDialog::ShowDirsOnly);
        videoFiles = getVideoFiles(videoPath);
    }

    std::cout << videoFiles.size() << " video files selected for processing" << std::endl;

    for (auto file: videoFiles) {
        if (cmdLine.isSet(roiOption)) {
            showRoi(file, params.detector.roi);
        }

        Error error = analyzeMotion(params, file);
        if (error != Error::OK) {
            std::cout << errorMsg.find(error)->second << std::endl;
        }
    }

    waitForEnter();
    return 0;
    /*
    rlutil::hidecursor();
    for (auto file : videoFiles) {
        for(int i=10; i<=100; i+=10)  {
            std::cout << file << "  " << i << "%\r";
            std::cout.flush();
            rlutil::msleep(200);
        }
        std::cout << file << "  ";
        rlutil::setColor(rlutil::GREEN);
        std::cout << "OK   " << std::endl;
        rlutil::resetColor();

        std::cout << file << "  ";
        rlutil::setColor(rlutil::RED);
        std::cout << "67%" << std::endl;
        rlutil::resetColor();
    }
    rlutil::showcursor();
    */

    /* check time
    time_t start = buffer.startTime();
    std::cout << std::put_time(std::localtime(&start), "%c") << std::endl;
    std::cout << buffer.timeStamp() << std::endl;
    //
    auto startChrono = std::chrono::system_clock::from_time_t(start);
    std::cout << "chrono: " << getTimeStamp(TimeResolution::sec_NoBlank, startChrono) << std::endl;
    auto addChrono = startChrono + std::chrono::milliseconds(600000);
    std::cout << "+600.000ms -> +10min"
              << getTimeStamp(TimeResolution::sec_NoBlank, addChrono) << std::endl;
    return Error::OK;
    */

    /*
    // check timezone
    const char* timeZone;
    timeZone = getenv("TZ");
    std::cout << "time zone: " << timeZone << std::endl;
    return 0;
    */
}
