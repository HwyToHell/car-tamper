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


bool waitForEnter() {
    using namespace std;
    cout << endl << "Press <enter> to exit" << endl;
    string str;
    getline(cin, str);
    return true;
}



int main(int argc, char *argv[])  {
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

    QString curPath = QDir::currentPath();
    qDebug() << curPath;

    std::vector<std::string> videoFiles;
    // take video files from command line
    if (posArgs.size() > 0) {
        // take all video files of current directory
        if (posArgs.at(0) == ".") {
            QString curPath = QDir::currentPath();
            videoFiles = getVideoFiles(curPath);

        // take each argument as video input file
        } else {
            for (auto arg : posArgs) {
                videoFiles.push_back(arg.toStdString());
            }
        }

    // select directory and extract all video files
    } else {
        QString videoPath = QFileDialog::getExistingDirectory(nullptr,
                        "Select Directory",
                        QDir::currentPath(),
                        QFileDialog::ShowDirsOnly);
        // TODO getVideoFiles must return absolute path (as opposed to relative path in current implementation)
        videoFiles = getVideoFiles(videoPath);
        for (auto file : videoFiles) {
            std::cout << file << std::endl;
        }
    }

    std::string videoFile = videoFiles.at(0);
    cv::VideoCapture cap;
    cap.open(videoFile);
    if (!cap.isOpened()) {
        std::cout << "cannot open video file: " << videoFile << std::endl;
        return -1;
    }

    cv::Mat frame;
    /*
    for (int i = 0; i < 20; ++i) {
        cap.read(frame);
        std::string file = "img_" + std::to_string(i) + ".jpg";
        bool success = cv::imwrite(file, frame);
    }
    */

    cv::namedWindow("video");
    while (cap.read(frame)) {
        cv::imshow("video", frame);
        if (cv::waitKey(10) == 27) {
            std::cout << "esc -> end video processing" << std::endl;
            break;
        }
    }
    cv::destroyWindow("video");


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
