#include "../inc/video-capture-simu.h"
#include <catch.hpp>


TEST_CASE("TAM-26 get / set", "[VideoCaptureSimu]") {
    VideoCaptureSimu vcs(InputMode::camera, "640x480");
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


TEST_CASE("TAM-25 read frame", "[VideoCaptureSimu]") {
    VideoCaptureSimu vcs(InputMode::camera, "640x480");
    SECTION("set and verify frame size output") {
        cv::Mat frame;
        const double height = 1080;
        REQUIRE(vcs.set(cv::CAP_PROP_FRAME_HEIGHT, height));
        REQUIRE(vcs.get(cv::CAP_PROP_FRAME_HEIGHT) == Approx(height));
        vcs.read(frame);
        vcs.read(frame);
        REQUIRE(frame.size().height == Approx(height));
    }
    SECTION("verify frame timing for camera mode") {
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

TEST_CASE("TAM-22 switch input mode", "[VideoCaptureSimu]") {
    // use videoFile input mode
    VideoCaptureSimu vcs(InputMode::videoFile, "160x120");
    vcs.get(cv::CAP_PROP_MODE);
    SECTION("verify video file input mode") {
        REQUIRE(vcs.get(cv::CAP_PROP_MODE) == Approx(static_cast<double>(InputMode::videoFile)));
    }
    SECTION("verify artificial frame rate has been set to 1000 fps") {
        REQUIRE(vcs.get(cv::CAP_PROP_FPS) == Approx(1000));
    }
}

TEST_CASE("TAM-28 verify generation mode", "[VideoCaptureSimu]") {
    std::string frameSize = "160x120";
    VideoCaptureSimu vcs(InputMode::videoFile, frameSize);
    SECTION("verify size and grey level of motion area") {
        int area_perCent = 50;
        int grayLevel_perCent = 50;
        vcs.setMode(GenMode::motionArea, area_perCent, grayLevel_perCent);

        // convert per cent to absolute
        int width = stoi(frameSize.substr(0, frameSize.find('x')));
        int height = stoi(frameSize.substr(frameSize.find('x')+1));
        int motionArea = width * height * area_perCent / 100;

        int grayLevel = UCHAR_MAX * grayLevel_perCent / 100;

        cv::Mat frame;
        vcs.read(frame);

        // setMode will be applied after another read handshake
        vcs.read(frame);
        // cv::imwrite("frame2.png", frame);

        // compare calculated motion area with counted pixels of motion area
        cv::Mat motionGray, motionBinary;
        cv::cvtColor(frame, motionGray, cv::COLOR_BGRA2GRAY);
        cv::inRange(motionGray, grayLevel-1, grayLevel+1, motionBinary);
        // add one as time stamp is encoded in first pixel of last row
        int motionAreaCounted = cv::countNonZero(motionBinary) + 1;
        REQUIRE(motionArea == motionAreaCounted);

        // compare calculated gray level with probed gray level of pixel
        int midCol = width / 2;
        int grayLevelProbed = static_cast<int>(motionGray.at<uchar>(0, midCol));
        REQUIRE(grayLevel == grayLevelProbed);
    }
}
