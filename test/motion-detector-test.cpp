#include "../inc/motionbuffer.h"
#include "../inc/motion-detector.h"
#include "../inc/video-capture-simu.h"
#include <catch.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>

#include <thread>

const std::string logDir("log");
const bool        isLogging = true;
const std::string videoDir("videos");

struct LogFiles{
    std::string logFileRelPath;
    std::string videoFileRelPath;
};

// defined in motionbuffer-test.cpp
std::vector<int> getBufferSamples(cv::FileStorage fs, std::string key);


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
        SECTION("roi not initialized -> use full frame when reading first one") {
            const int width = 640;
            const int height = 480;
            const cv::Rect fullFrame(0, 0, width, height);
            cv::Mat frame(height, width, CV_8U) ;
            REQUIRE(det.roi() == cv::Rect(0, 0, 0, 0));
            det.hasFrameMotion(frame);
            REQUIRE(det.roi() == fullFrame);
        }
        SECTION("set an get roi") {
            const cv::Rect roi(0,0,640,480);
            det.roi(roi);
            REQUIRE(det.roi() == roi);
        }
    }
} // TEST_CASE TAM-17 set / get

// verify continous motion detection
TEST_CASE("TAM-37 isContinuousMotion", "[MotionDetector][TAM-37]") {
    const size_t bufferSize = 30;
    const size_t fps = 30;
    const int minMotionDuration = 10;   // 10 consecutive frames
    const int minMotionIntensity = 20;  // 20% of frame area
    const double threshold = 50;        // frame difference threshold

    const int startMotion = 30;         // count for motion start
    const int startSaveToDisk = startMotion + minMotionDuration;
    const int stopMotion = 60;
    const int stopSaveToDisk = stopMotion + minMotionDuration;
    const int stopReadingCam = 110;

    VideoCaptureSimu vcs(InputMode::camera, "640x480", fps);
    MotionBuffer buf(bufferSize, fps, videoDir, logDir, isLogging);
    MotionDetector det;

    // use full frame size of camera source as roi
    int frameWidth = static_cast<int>(vcs.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(vcs.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::Rect roi(0, 0, frameWidth, frameHeight);
    det.roi(roi);
    det.bgrSubThreshold(threshold);
    det.minMotionDuration(minMotionDuration);
    det.minMotionIntensity(minMotionIntensity);

    cv::Mat frame;
    for (int count = 0; count < stopReadingCam; ++count) {
        vcs.read(frame);
        buf.pushToBuffer(frame);
        bool isMotion = det.isContinuousMotion(frame);

        if (isMotion) {
            if (!buf.isSaveToDiskRunning()) {
                buf.setSaveToDisk(true);
            }
        } else {
            buf.setSaveToDisk(false);
        }

        if (count == startMotion) {
            vcs.setMode(GenMode::motionAreaAndTime, minMotionIntensity+1, 50);
        }

        if (count == stopMotion) {
            vcs.setMode(GenMode::timeStamp);
        }
    }

    // wait for post buffer to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // release necessary to finish post buffer and close video file
    buf.releaseBuffer();

    LogFiles files;
    files.logFileRelPath = buf.getLogFileRelPath();
    files.videoFileRelPath = videoDir + '/' + buf.getVideoFileName();
    std::cout << "new video file created: " << files.videoFileRelPath << std::endl;

    cv::FileStorage fs(files.logFileRelPath, cv::FileStorage::Mode::READ);
    std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

    // pre-buffer + (stopSaveToDisk - startSaveToDisk) + post-buffer
    size_t motionFrames = bufferSize + (stopSaveToDisk - startSaveToDisk) + bufferSize;
    REQUIRE(frmCounts.size() == motionFrames);

} // TEST_CASE TAM-37 isContinuousMotion

// clean up
TEST_CASE("delete temporary files", "[MotionDetector][TearDown]") {
    std::string cwd = cv::utils::fs::getcwd();
    cv::utils::fs::remove_all(cwd + "/" + logDir);
    cv::utils::fs::remove_all(cwd + "/" + videoDir);
    std::cout << "temp dirs cleaned up" << std::endl;
}


