#include "../inc/video-capture-simu.h"
#include <catch.hpp>


TEST_CASE("TAM-26 get / set", "[VidCapSimu]") {
    VideoCaptureSimu vcs;
    SECTION("set fps in range") {
        const double fpsInRange = 20;
        vcs.set(cv::CAP_PROP_FPS, fpsInRange);
        REQUIRE(vcs.get(cv::CAP_PROP_FPS) == Approx(fpsInRange));
    }
    SECTION("set fps larger than max") {
        const double fpsMax = 60;
        vcs.set(cv::CAP_PROP_FPS, 100);
        REQUIRE(vcs.get(cv::CAP_PROP_FPS) == Approx(fpsMax));
    }
    SECTION("set fps larger than max") {
        const double fpsMin = 1;
        vcs.set(cv::CAP_PROP_FPS, 0);
        REQUIRE(vcs.get(cv::CAP_PROP_FPS) == Approx(fpsMin));
    }
    SECTION("set frame width in range") {
        const double width = 320;
        REQUIRE(vcs.set(cv::CAP_PROP_FRAME_WIDTH, width));
        REQUIRE(vcs.get(cv::CAP_PROP_FRAME_WIDTH) == Approx(width));
    }
    SECTION("set frame width out of range") {
        const double width = 888;
        REQUIRE(!vcs.set(cv::CAP_PROP_FRAME_WIDTH, width));
    }
    SECTION("set frame height in range") {
        const double height = 768;
        REQUIRE(vcs.set(cv::CAP_PROP_FRAME_HEIGHT, height));
        REQUIRE(vcs.get(cv::CAP_PROP_FRAME_HEIGHT) == Approx(height));
    }
}


TEST_CASE("TAM-25 read frame", "[VidCapSimu]") {
    VideoCaptureSimu vcs;
    SECTION("set and verify frame size output") {
        cv::Mat frame;
        const double height = 1080;
        REQUIRE(vcs.set(cv::CAP_PROP_FRAME_HEIGHT, height));
        REQUIRE(vcs.get(cv::CAP_PROP_FRAME_HEIGHT) == Approx(height));
        vcs.read(frame);
        vcs.read(frame);
        REQUIRE(frame.size().height == Approx(height));
    }
    SECTION("verify frame timing") {
        const double fps = 60;
        vcs.set(cv::CAP_PROP_FPS, fps);
        REQUIRE(vcs.get(cv::CAP_PROP_FPS) == Approx(fps));
        cv::Mat frame;
        auto start = std::chrono::system_clock::now();
        auto end  = std::chrono::system_clock::now();
        const int nLoops = 1;
        for (int i = 0; i <= nLoops; ++i) {
            vcs.read(frame);
            if (i == 0) {
                start = std::chrono::system_clock::now();
            }
            if (i == 1) {
                end = std::chrono::system_clock::now();
            }
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        // allow for 10% deviation
        REQUIRE(elapsed.count() == Approx(1000 / fps).epsilon(0.1));
        //std::cout << "elapsed ms: " << elapsed.count() << std::endl;
    }
    SECTION("verify frame count is encoded in first pixel") {
        cv::Mat frame;
        vcs.read(frame);
        REQUIRE(frame.at<int>(0) == 0);
        vcs.read(frame);
        REQUIRE(frame.at<int>(0) == 1);
    }
}

TEST_CASE("TAM-22 switch input mode", "[VidCapSimu]") {
    VideoCaptureSimu vcs;
    SECTION("verify camera input mode") {
}
