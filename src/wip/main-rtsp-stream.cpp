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
    /* file exists */
    namespace fs = std::experimental::filesystem;
    fs::path videoPath(fileName);
    if (!fs::exists(videoPath))
        return Error::FileExist;

    /* parse time from file name */
    videoPath.replace_extension();
    std::stringstream ss;

    // convert path to string to omit double quotes and avoid get_time parsing error
    ss << videoPath.filename().string();
    std::tm startTime{};
    ss >> std::get_time(&startTime, "%Y-%m-%d_%Hh%Mm%Ss");
    if (ss.fail())
        return Error::ParseTime;

    // isdst = -1 --> determine DST and time zone via maketime
    startTime.tm_isdst = -1;
    std::mktime(&startTime);

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

    // DEBUG
    /*
    cv::namedWindow("motion mask", cv::WINDOW_NORMAL);
    cv::resizeWindow("motion mask", 900, 300);
    cv::moveWindow("colored mask", 0, 0);

    cv::namedWindow("colored mask", cv::WINDOW_NORMAL);
    cv::resizeWindow("colored mask", 900, 300);
    cv::moveWindow("colored mask", 1000, 0);
    */
    // DEBUG_END

    int frameCount = 0;
    int progress = 0;
    cv::Mat frame;
    rlutil::saveDefaultColor();
    rlutil::CursorHider hide;
    while (cap.read(frame)) {
        ++frameCount;
        buffer.pushToBuffer(frame);
        bool isMotion = detector.isContinuousMotion(frame);

        // DEBUG
        /*
        if (cv::waitKey(10) == 32) 	{
            std::cout << "SPACE pressed -> continue by hitting SPACE again, ESC to abort processing" << std::endl;
            int key = cv::waitKey(0);
            if (key == 27)
                break;
        }
        */
        // DEBUG_END

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
        rlutil::setColor(10); // green
        std::cout << "OK   " << std::endl;
        rlutil::resetColor();
        return Error::OK;
    } else {
        std::cout << fileName << "  ";
        rlutil::setColor(12); // red
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


void statistics(std::vector<long long> samples, std::string name)
{
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wall"
    // std::vector<long long>::iterator itMax = std::max_element(samples.begin(), samples.end());
    long long max = *(std::max_element(samples.begin(), samples.end()));
    // long long max = *itMax;
    #pragma GCC diagnostic pop

    long long sum = std::accumulate(samples.begin(), samples.end(), 0);
    double mean = static_cast<double>(sum) / samples.size();

    double variance = 0, stdDeviation = 0;
    for (auto sample : samples) {
        variance += pow ((sample - mean), 2);
    }
    stdDeviation = sqrt(variance / samples.size());

    std::cout << std::endl << "===================================" << std::endl << name << std::endl;
    std::cout << "mean: " << mean << " max: " << max << std::endl;
    std::cout << "95%:  " << mean + (2 * stdDeviation) << std::endl;
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
int main_rtsp_stream(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("grzonka");
    // settings file
    // linux:   ~/.config/grzonka/tamper.conf
    // windows: HKEY_CURRENT_USER\Software\grzonka\tamper
    Params params;

    // parse command line args
    QCommandLineParser cmdLine;
    cmdLine.setApplicationDescription("Extract motion sequences of video file to separate directories");
    cmdLine.addHelpOption();
    QCommandLineOption roiOption(QStringList() << "r" << "roi", "show roi before processing files");
    cmdLine.addOption(roiOption);
    cmdLine.addPositionalArgument("file", "Process video file(s), separated by blanks");
    cmdLine.addPositionalArgument(".", "Process all video files in current directory");
    cmdLine.addPositionalArgument("", "Select video files to process");

    cmdLine.process(a);
    const QStringList posArgs = cmdLine.positionalArguments();

    //cv::VideoCapture cap("rtsp://admin:@192.168.1.10");
    cv::VideoCapture capFile("2021-05-10_14h09m00s.mp4");
    cv::Mat frame;
    int frameCount = 0;

    //cv::namedWindow("stream", cv::WINDOW_NORMAL);
    //cv::resizeWindow("stream", 900, 300);
    //cv::moveWindow("stream", 0, 0);

    // analyze file
    std::string outDirectory("motion");
    MotionBuffer buffer(params.buffer.pre,
                        25,
                        outDirectory,
                        "log",
                        false,
                        false);
    buffer.postBuffer(params.buffer.post);
    // buffer.startTime(startTime);

    MotionDetector detector;
    detector.bgrSubThreshold(params.detector.bgrSubThreshold);
    detector.minMotionDuration(params.detector.minMotionDuration);
    detector.minMotionIntensity(params.detector.minMotionIntensity);
    detector.roi(cv::Rect(0,0,0,0));

    // measurement
    std::vector<long long> readTimes;
    std::vector<long long> pushTimes;
    std::vector<long long> motionTimes;

    capFile.read(frame);
    while (true) {
        // TODO time measurement
        TimePoint start = std::chrono::system_clock::now();

        capFile.read(frame);
        TimePoint read = std::chrono::system_clock::now();

        buffer.pushToBuffer(frame);
        TimePoint push = std::chrono::system_clock::now();

        bool isMotion = detector.isContinuousMotion(frame);
        if (isMotion) {
            if (!buffer.isSaveToDiskRunning()) {
                buffer.setSaveToDisk(true);
            }
        } else {
            buffer.setSaveToDisk(false);
        }

        TimePoint motion = std::chrono::system_clock::now();

        ++frameCount;
        //cv::imshow("stream", frame);

        // std::cout << "frame size: " << frame.size << std::endl;
        long long readDuration = std::chrono::duration_cast<std::chrono::milliseconds>(read - start).count();
        readTimes.push_back(readDuration);

        long long pushDuration = std::chrono::duration_cast<std::chrono::milliseconds>(push - read).count();
        pushTimes.push_back(pushDuration);

        long long motionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(motion - push).count();
        motionTimes.push_back(motionDuration);

        std::cout << "frame " << frameCount
            << ": read " << readDuration
            << ", push " << pushDuration
               << ", motion " << motionDuration
            << std::endl;


        if (cv::waitKey(10) == 32) 	{
            std::cout << "SPACE pressed -> continue by hitting SPACE again, ESC to abort processing" << std::endl;
            int key = cv::waitKey(0);
            if (key == 27)
                break;
        }
        if (frameCount >= 250)
            break;
    }
    //cv::destroyWindow("stream");
    statistics(readTimes, "read in ms");
    statistics(pushTimes, "push in ms");
    statistics(motionTimes, "motion in ms");


    //waitForEnter();
    return 0;
}
