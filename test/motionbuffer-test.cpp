#include "../inc/motionbuffer.h"
#include "../inc/video-capture-simu.h"
#include <catch.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;
const std::string logDir("log");
const bool        isLogging = true;
const std::string videoDir("videos");

struct LogFiles{
    std::string logFileRelPath;
    std::string videoFileRelPath;
};


/*
 * get value (time series) of a given key
 * fs: logfile (.json)
 * key: variable to extract samples from
 * returns vector of values
 */
std::vector<int> getBufferSamples(cv::FileStorage fs, std::string key) {
    std::vector<int> bufSamples;
    cv::FileNode root = fs.root();
    if (root.size() < 1) {
        std::cout << "root file node empty" << std::endl;
    }

    // key available?
    cv::FileNodeIterator itFirstNode = root.begin();
    std::vector<cv::String> keys = (*itFirstNode).keys();
    bool isKeyFound = false;
    for (auto k : keys) {
        if (k.compare(key) == 0) {
            isKeyFound = true;
        }
    }
    if (!isKeyFound) {
        std::cout << "file node does not contain key: " << key << std::endl;
        return bufSamples;
    }

    // populate vector with values associated with key
    for (cv::FileNodeIterator itSample = root.begin(); itSample != root.end(); ++itSample) {
        bufSamples.push_back(static_cast<int>((*itSample)[key]));
    }

    return bufSamples;
}


/*
 * returns video file name
 * start: enable saveToDisk frame number
 * stop: disable saveToDisk frame number
 */
LogFiles writeToDiskTest(VideoCaptureSimu& cap, size_t bufSize, double fpsOut,
                     int start, int stop, int overrun) {

    LogFiles logFiles;
    logFiles.logFileRelPath = logFiles.videoFileRelPath = "error";
    if (start >= stop || stop >= overrun) {
        std::cout << "start < stop < overrun" << std::endl;
        return logFiles;
    }

    MotionBuffer buf(bufSize, fpsOut, videoDir, logDir, true);
    cv::Mat frame;
    for (int count = 0; count < overrun; ++count) {
        std::cout << std::endl << "pass: " << count << std::endl;

        // buf = 1 (min value) -> first frame written = start+1
        if (count >= start && !buf.isSaveToDiskRunning())
            buf.setSaveToDisk(true);

        // stop must be start+1 or greater to generate log file
        if (count >= stop)
            buf.setSaveToDisk(false);

        cap.read(frame);
        std::cout << "frame read" << std::endl;
        buf.pushToBuffer(frame);
    }

    logFiles.logFileRelPath = buf.getLogFileRelPath();
    logFiles.videoFileRelPath = videoDir + '/' + buf.getVideoFileName();
    std::cout << "new video file created: " << logFiles.videoFileRelPath << std::endl;
    return logFiles;
}



// buffer size verification: determine number of written frames from logFile
// make sure to turn on logging in VideoCaptureSimu
// number of logged frames = buffer size + (stop - start)
TEST_CASE("TAM-18 constructor: buffer size", "[MotionBuffer]") {
    const size_t minBufSize = 1;
    const size_t maxBufSize = 60;

    std::string cwd = cv::utils::fs::getcwd();
    const size_t fps = 30;
    VideoCaptureSimu vcs(InputMode::camera, "160x120", fps, false);

    SECTION("preBufferSize below min") {
        const size_t bufSizeBelow = 0;
        const int startFrame = minBufSize + 10;

        LogFiles files = writeToDiskTest(vcs, bufSizeBelow, fps,
                                          startFrame, startFrame+1, startFrame+5);

        cv::FileStorage fs(files.logFileRelPath, cv::FileStorage::Mode::READ);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        REQUIRE(frmCounts.size() == minBufSize + 1);
    }

    SECTION("preBufferSize in range") {
        const size_t bufSizeInRange = 30;
        const int startFrame = bufSizeInRange + 10;

        LogFiles files = writeToDiskTest(vcs, bufSizeInRange, fps,
                                          startFrame, startFrame+1, startFrame+5);

        cv::FileStorage fs(files.logFileRelPath, cv::FileStorage::Mode::READ);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        // bufferSize + 1 frames logged (if frame rate is sufficiently low)
        REQUIRE(frmCounts.size() == bufSizeInRange + 1);
    }

    SECTION("preBufferSize above max, verify frame rate") {
        const size_t bufSizeAbove = 100;

        MotionBuffer mb(bufSizeAbove, fps, videoDir, logDir, isLogging);
        const int startFrame = maxBufSize + 10;
        LogFiles files = writeToDiskTest(vcs, bufSizeAbove, fps,
                                          startFrame, startFrame+1, startFrame+5);

        cv::FileStorage fs(files.logFileRelPath, cv::FileStorage::Mode::READ);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        // bufferSize + 1 frames logged (if frame rate is sufficiently low)
        REQUIRE(frmCounts.size() == maxBufSize + 1);

        // verify frame count increases monotonic
        for (size_t n = 1; n < frmCounts.size(); ++n) {
            REQUIRE((frmCounts[n] - frmCounts[n-1]) > 0);
        }

        // verify frame rate matches specification and stays the same
        std::vector<int> timeStamps = getBufferSamples(fs, "time stamp");
        double frameTimePrev = static_cast<double>(timeStamps[1] - timeStamps[0]);
        for (size_t n = 2; n < timeStamps.size(); ++n) {
            double frameTime = static_cast<double>(timeStamps[n] - timeStamps[n-1]);
            REQUIRE(frameTime == Approx(1000 / fps).epsilon(0.1));
            std::cout << "frame rate: " << frameTime << std::endl;
            REQUIRE(frameTimePrev == Approx(frameTime).epsilon(0.1));
            frameTimePrev = frameTime;
        }
    }
}



// verify buffer size and monotony in all test sections
TEST_CASE("TAM-16 read frame in video file input mode", "[MotionBuffer]") {
    std::string cwd = cv::utils::fs::getcwd();
    VideoCaptureSimu vcs(InputMode::videoFile, "160x120", 30, false);

    SECTION("buffer full when starting saveMotionToDisk, "
            "sufficient new frames to empty buffer before destruction") {
        const size_t bufSize = 30;
        const double fpsOutput = 30;
        const int startFrame = bufSize + 10;

        // large overrun, plenty of time to finish post buffer
        LogFiles files = writeToDiskTest(vcs, bufSize, fpsOutput,
                                          startFrame, startFrame+1, startFrame+200);

        cv::FileStorage fs(files.logFileRelPath, cv::FileStorage::Mode::READ);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        // for lager overruns: bufSize + 1
        REQUIRE(frmCounts.size() == bufSize + 1);

        // verify frame count increases monotonic
        for (size_t n = 1; n < frmCounts.size(); ++n) {
            REQUIRE((frmCounts[n] - frmCounts[n-1]) > 0);
        }
    }

    SECTION("buffer half full when starting saveMotionToDisk") {
        const size_t bufSize = 30;
        const double fpsOutput = 30;

        // smaller buffer size
        const int startFrame = bufSize - 15;

        // overrun ends immediately
        LogFiles files = writeToDiskTest(vcs, bufSize, fpsOutput,
                                          startFrame, startFrame+1, startFrame+50);

        cv::FileStorage fs(files.logFileRelPath, cv::FileStorage::Mode::READ);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        REQUIRE(frmCounts.size() == bufSize + 1);

        // verify frame count increases monotonic
        for (size_t n = 1; n < frmCounts.size(); ++n) {
            REQUIRE((frmCounts[n] - frmCounts[n-1]) > 0);
        }
    }
}


// compare set fpsOutput of motionBuffer with read fps of output file
// VideoCapture in camera mode
void verifyFps (size_t bufSize, size_t fpsSource, double fpsOutSet, double fpsOutTest,
                std::string frameSize) {
    VideoCaptureSimu vcs(InputMode::camera, frameSize, fpsSource);
    cv::VideoCapture cap;

    const int startFrame = static_cast<int>(bufSize);
    const int length = static_cast<int>(bufSize);

    // overrun must be big enough for post buffer to finish
    const int overrun = static_cast<int>(bufSize) + 10;
    LogFiles files = writeToDiskTest(vcs, bufSize, fpsOutSet,
                                         startFrame,
                                         startFrame + length,
                                         startFrame + length + overrun);

    double width = stod(frameSize.substr(0, frameSize.find('x')));
    double height = stod(frameSize.substr(frameSize.find('x')+1));

    cap.open(files.videoFileRelPath);
    REQUIRE(cap.isOpened() == true);
    REQUIRE(cap.get(cv::CAP_PROP_FPS) == Approx(fpsOutTest).epsilon(0.01));
    std::cout << "video file with " << cap.get(cv::CAP_PROP_FPS) << ": "
              << files.videoFileRelPath << std::endl;
    REQUIRE(cap.get(cv::CAP_PROP_FRAME_WIDTH) == Approx(width));
    REQUIRE(cap.get(cv::CAP_PROP_FRAME_HEIGHT) == Approx(height));
    cap.release();
}



/*
// fps verification: re-read written output video, get fps and frame size
TEST_CASE("TAM-20 constructor: fps", "[MotionBuffer]") {
    const size_t sourceFps = 60;

    SECTION("smallest frame size") {
        std::string smallestFrame("160x120");

        SECTION("fps below min") {
            const double fpsBelowMin = 0;
            const double fpsMinTest = 1;
            const size_t bufferSize = 30;
            verifyFps(bufferSize, sourceFps, fpsBelowMin, fpsMinTest, smallestFrame);
        }

        SECTION("fps in range") {
            const double fpsInRange = 30;
            const size_t bufferSize = 30;
            verifyFps(bufferSize, sourceFps, fpsInRange, fpsInRange, smallestFrame);
        }

        SECTION("fps above max") {
            const double fpsAboveMax = 100;
            const double fpsMaxTest = 60;
            const size_t bufferSize = 30;
            verifyFps(bufferSize, sourceFps, fpsAboveMax, fpsMaxTest, smallestFrame);
        }
    }
}
*/


// clean up
TEST_CASE("delete temporary files", "[TearDown]") {
    std::string cwd = cv::utils::fs::getcwd();
    cv::utils::fs::remove_all(cwd + "/" + logDir);
    cv::utils::fs::remove_all(cwd + "/" + videoDir);
}

