#include "../inc/motionbuffer.h"
#include "../inc/motion-detector.h"
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



// clean up
TEST_CASE("TAM-31 delete temporary files", "[MotionBuffer][TearDown]") {
    std::string cwd = cv::utils::fs::getcwd();
    cv::utils::fs::remove_all(cwd + "/" + logDir);
    cv::utils::fs::remove_all(cwd + "/" + videoDir);
}

