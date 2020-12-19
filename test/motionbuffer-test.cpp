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


/*
 * returns video file name
 * start: enable saveToDisk frame number
 * stop: disable saveToDisk frame number
 */
std::string writeToDiskTest(MotionBuffer& buf, VideoCaptureSimu& cap,
                     int start, int stop) {
    cv::Mat frame;
    for (int count = 0; count < stop+5; ++count) {
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

    // TODO if saveToDiskRunning -> use blocking version of getMotionFileName
    std::string videoFileName = buf.getVideoFileName();
    std::cout << "new video file created: " << videoFileName << std::endl;
    return videoFileName;
}


// TODO define logDir at central location and use this variable in all test cases
// TODO clean up logDir after test have been executed successfully


/* buffer size verification: determine number of written frames from logFile */
TEST_CASE("#mb001 constructor: buffer size", "[MotionBuffer]") {
    const size_t minBufSize = 1;
    const size_t maxBufSize = 60;

    std::string cwd = cv::utils::fs::getcwd();
    const std::string logDir("log");
    const std::string videoDir("videos");

    /* frame size 160x120 and frame rate 10 fps should be enough
     * to empty buffer in one cycle
     * check DEBUG output, if this is the case
     */
    const size_t fps = 10;
    VideoCaptureSimu vcs(fps, "160x120");

    /* number of logged frames = (buffer size - 1) + (stop - start - 1),
     *                            frames in buffer      serial frames
     * for low source frame rates that allow writing all buffer content
     * during one cycle waiting for the next frame to read
     *
     * for higher source frame rates excess frames are written to file
     * due to writing process not finished when new frame is read
     *
     * if buffer size == 1 -> one frame in buffer logged
     */

    SECTION("preBufferSize below min") {
        const size_t bufSizeBelow = 0;

        MotionBuffer mb(bufSizeBelow, fps, videoDir, logDir);
        const int startFrame = minBufSize + 10;
        std::string videoFile = writeToDiskTest(mb, vcs, startFrame, startFrame+1);
        std::string logFileRelPath = mb.getLogFileRelPath();

        cv::FileStorage fs(logFileRelPath, cv::FileStorage::Mode::read);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        /* for buffer size one -> one frame is logged */
        REQUIRE(frmCounts.size() == minBufSize);

    }

    SECTION("preBufferSize in range") {
        const size_t bufSizeInRange = 30;

        MotionBuffer mb(bufSizeInRange, fps,  videoDir, logDir);
        const int startFrame = bufSizeInRange + 10;
        writeToDiskTest(mb, vcs, startFrame, startFrame+1);
        std::string logFileRelPath = mb.getLogFileRelPath();
        cv::FileStorage fs(logFileRelPath, cv::FileStorage::Mode::read);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        /* bufferSize - 1 frames logged (if frame rate is sufficiently low) */
        REQUIRE(frmCounts.size() == bufSizeInRange - 1);
    }

    SECTION("preBufferSize above max") {
        const size_t bufSizeAbove = 100;

        MotionBuffer mb(bufSizeAbove, fps,  videoDir, logDir);
        const int startFrame = maxBufSize + 10;
        writeToDiskTest(mb, vcs, startFrame, startFrame+1);
        std::string logFileRelPath = mb.getLogFileRelPath();
        cv::FileStorage fs(logFileRelPath, cv::FileStorage::Mode::read);
        std::vector<int> frmCounts = getBufferSamples(fs, "frame count");

        /* bufferSize - 1 frames logged (if frame rate is sufficiently low) */
        REQUIRE(frmCounts.size() == maxBufSize - 1);

        /* frame count increases monotonic */
        for (size_t n = 1; n < frmCounts.size(); ++n) {
            REQUIRE((frmCounts[n] - frmCounts[n-1]) > 0);
        }

        /* frame rate matches specification and stays the same */
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

    //cv::utils::fs::remove_all(cwd + "/" + videoDir);
} /* TEST_CASE "#mb001 constructor: buffer size" */


/*

// fps verification: re-read written output video and get frame size
TEST_CASE("#mb002 constructor: fps", "[MotionBuffer]") {
    const double fpsMin = 1;
    const double fpsMax = 60;

    const std::string logDir("logDir");
    const size_t bufferSize = 10;
    const size_t sourceFps = 30;
    VideoCaptureSimu vcs(sourceFps, "160x120");
    cv::VideoCapture cap;

    SECTION("fps below min") {
        const double fpsBelowMin = 0;

        MotionBuffer mb(bufferSize, fpsBelowMin, logDir);
        const int startFrame = bufferSize + 5;
        const int length = 10;
        std::string videoFile = writeToDiskTest(mb, vcs, startFrame, startFrame + length);

        cap.open(videoFile);
        REQUIRE(cap.isOpened());
        REQUIRE(cap.get(cv::CAP_PROP_FPS) == Approx(fpsMin).epsilon(0.01));
        std::cout << "video file with " << cap.get(cv::CAP_PROP_FPS) << ": "
                  << videoFile << std::endl;
    }

    SECTION("fps in range") {
        const double fpsInRange = 0;

        MotionBuffer mb(bufferSize, fpsInRange, logDir);
        const int startFrame = bufferSize + 5;
        const int length = 30;
        std::string videoFile = writeToDiskTest(mb, vcs, startFrame, startFrame + length);

        cap.open(videoFile);
        REQUIRE(cap.isOpened());
        REQUIRE(cap.get(cv::CAP_PROP_FPS) == Approx(fpsMin).epsilon(0.01));
        std::cout << "video file with " << cap.get(cv::CAP_PROP_FPS) << ": "
                  << videoFile << std::endl;
    }

    SECTION("fps above max") {
        const double fpsAboveMax = 100;

        MotionBuffer mb(bufferSize, fpsAboveMax, logDir);
        const int startFrame = bufferSize + 5;
        const int length = 60;
        std::string videoFile = writeToDiskTest(mb, vcs, startFrame, startFrame + length);

        cap.open(videoFile);
        REQUIRE(cap.isOpened());
        REQUIRE(cap.get(cv::CAP_PROP_FPS) == Approx(fpsMax).epsilon(0.01));
        std::cout << "video file with " << cap.get(cv::CAP_PROP_FPS) << ": "
                  << videoFile << std::endl;
    }

    // TODO clean up logDir after executing tests
}

*/

TEST_CASE("#mb002 save to disk", "[MotionBuffer]") {

    // verify: frames order, timing, video output directory, video output file name


}


