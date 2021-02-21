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


// verify parameters have been set correctly by re-reading them
TEST_CASE("TAM-17 set / get parameters", "[MotionDetector][TAM-17]") {
    MotionDetector det;

    SECTION("background subtractor threshold") {
        // 0 ... 100
        const double bgrMin = 0;
        const double bgrMax = 100;

        SECTION("below range") {
            const double bgrBelowRange = -1;
            det.bgrSubThreshold(bgrBelowRange);
            REQUIRE(det.bgrSubThreshold() == Approx(bgrMin));
        }
        SECTION("in range") {
            const double bgrInRange = 50;
            det.bgrSubThreshold(bgrInRange);
            REQUIRE(det.bgrSubThreshold() == Approx(bgrInRange));
        }
        SECTION("above range") {
            const double bgrAboveRange = 200;
            det.bgrSubThreshold(bgrAboveRange);
            REQUIRE(det.bgrSubThreshold() == Approx(bgrMax));
        }
    }

    SECTION("min motion duration") {
        // 0 ... 300
        const int durationMin = 0;
        const int durationMax = 300;

        SECTION("below range") {
            const int durationBelowRange = -1;
            det.minMotionDuration(durationBelowRange);
            REQUIRE(det.minMotionDuration() == durationMin);
        }
        SECTION("in range") {
            const int durationInRange = 50;
            det.minMotionDuration(durationInRange);
            REQUIRE(det.minMotionDuration() == durationInRange);
        }
        SECTION("above range") {
            const int durationAboveRange = 400;
            det.minMotionDuration(durationAboveRange);
            REQUIRE(det.minMotionDuration() == durationMax);
        }
    }

    SECTION("min motion intensity") {
        // 0 ... 100
        const int intensityMin = 0;
        const int intensityMax = 100;

        SECTION("below range") {
            const int intensityBelowRange = -1;
            det.minMotionIntensity(intensityBelowRange);
            REQUIRE(det.minMotionIntensity() == intensityMin);
        }
        SECTION("in range") {
            const int intensityInRange = 50;
            det.minMotionIntensity(intensityInRange);
            REQUIRE(det.minMotionIntensity() == intensityInRange);
        }
        SECTION("above range") {
            const int intensityAboveRange = 400;
            det.minMotionIntensity(intensityAboveRange);
            REQUIRE(det.minMotionIntensity() == intensityMax);
        }
    }

    SECTION("roi") {
        const cv::Rect roi(0,0,640,480);
        det.roi(roi);
        REQUIRE(det.roi() == roi);
    }
} // TEST_CASE TAM-17 set / get


