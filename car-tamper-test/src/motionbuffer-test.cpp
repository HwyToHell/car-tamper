#include "../inc/motionbuffer.h"
#include "../inc/video-capture-simu.h"
#include <catch.hpp>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;


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


// returns relative path to log file
std::string writeToDiskTest(MotionBuffer& buf, VideoCaptureSimu& cap,
                     int start, int stop) {
    cv::Mat frame;
    for (int count = 1; count < stop+3; ++count) {
        std::cout << std::endl << "pass: " << count << std::endl;

        // buf = 1 (min value) -> first frame written = start+1
        if (count >= start && !buf.isSaveToDiskRunning())
            buf.setSaveToDisk(true);
        // stop must be start+2 or greater to generate log file
        if (count >= stop)
            buf.setSaveToDisk(false);

        cap.read(frame);
        std::cout << "frame read" << std::endl;
        buf.pushToBuffer(frame);
    }
    std::this_thread::sleep_for(100ms);
    REQUIRE(buf.isNewMotionFile());
    return buf.getLogFileRelPath();
}



TEST_CASE("#mb001 construct", "[MotionBuffer]") {
    const std::string logDir("logDir");
    const size_t fps = 30;
    VideoCaptureSimu vcs(fps);

    // buffer size verification: determine number of written frames from logFile
    SECTION("preBufferSize below min") {
        MotionBuffer mb(2, fps, logDir);
        std::string logFileRelPath = writeToDiskTest(mb, vcs, 5, 6);
        std::cout << "writeToDisk finished" << std::endl;

        cv::FileStorage fs(logFileRelPath, cv::FileStorage::Mode::READ);
        REQUIRE(fs.isOpened());
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");


        // min buffer size = 1
        //REQUIRE(frmCounts.size() == 1);


    }
    SECTION("preBufferSize above max") {

    }
    SECTION("preBufferSize in range") {

    }

    // fps verification: re-read written output video and get frame size
    SECTION("fps below min") {
        const double fpsBelowMin = 0;
        MotionBuffer mb(1, fpsBelowMin, logDir);

        REQUIRE(true == true);
    }
    SECTION("fps above max") {

    }
    SECTION("fps in range") {

    }
}

TEST_CASE("#mb002 save to disk", "[MotionBuffer]") {

    // verify: frames order, timing, video output directory, video output file name

}


// clean up logDir after executing tests
