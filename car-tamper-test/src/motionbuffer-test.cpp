#include "../inc/motionbuffer.h"
#include <catch.hpp>


TEST_CASE("#mb001 construct", "[MotionBuffer]") {
    const std::string logDir("logDir");

    // buffer size verification: determine number of written frames from logFile
    SECTION("preBufferSize below min") {

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
