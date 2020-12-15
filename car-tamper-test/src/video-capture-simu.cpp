#include "../inc/video-capture-simu.h"
#include "../../inc/motionbuffer.h"
#include "../../inc/time-stamp.h"

#include <cassert>
#include <functional>
#include <sstream>


VideoCaptureSimu::VideoCaptureSimu(size_t framesPerSecond, std::string videoSize) :
    m_availVideoSizes{
        {"160x120", cv::Size(160,120)},
        {"320x240", cv::Size(320,240)},
        {"640x480", cv::Size(640,480)},
        {"1024x768", cv::Size(1024,768)},
        {"1920x1080", cv::Size(1920,1080)}
    },
    m_cntFrame{0},
    m_fps{framesPerSecond},
    m_frameSize{m_availVideoSizes.find("640x480")->second},
    m_isLogging{false},
    m_isNewFrame{false},
    m_isReleased{false}

{
    selectVideoSize(videoSize);
    m_thread = std::thread(&VideoCaptureSimu::generateFrame, this);
}


VideoCaptureSimu::~VideoCaptureSimu() {
    release();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    assert(!m_thread.joinable());
}

/*
 * return synthetic frame with time stamp in ms
 */
cv::Mat frameTimeStamp(cv::Size frameSize) {
    const cv::Scalar black  = cv::Scalar(0,0,0);
    const cv::Scalar red    = cv::Scalar(0,0,255);
    cv::Mat frame(frameSize, CV_8UC3, black);
    std::string timeStamp = getTimeStamp(TimeResolution::ms);
    cv::putText(frame, timeStamp, cv::Point(10,50),
                cv::FONT_HERSHEY_SIMPLEX, 1, red, 2);
    return frame;
}


/*
 * generate synthetic frame according to frame rate in concurrent thread
 * new frame with printed time stamp watermark, frame count encoded in pixel(0,0)
 * and time stamp (ms) as int in pixel(0,height)
 * main thread notified after new frame available
 */
void VideoCaptureSimu::generateFrame() {
    auto durationPerFrame = std::chrono::milliseconds(1000 / m_fps);
    if (m_isLogging) {
        std::cout << "frame duration: " << durationPerFrame.count() << std::endl;
    }

    std::chrono::system_clock::time_point startTimePoint = std::chrono::system_clock::now();

    while (!m_isReleased) {
        {
            std::lock_guard<std::mutex> newFrameLock(m_mtxNewFrame);
            m_sourceFrame = frameTimeStamp(m_frameSize);
            durationPerFrame = std::chrono::milliseconds(1000 / m_fps);

            // encode frame count as int in first pixel
            m_sourceFrame.at<int>(0,0) = m_cntFrame;

            // encode time stamp in ms as int in first pixel of last row
            m_sourceFrame.at<int>(0,m_sourceFrame.rows-1) = static_cast<int>(elapsedMs(startTimePoint));

            m_isNewFrame = true;
            ++m_cntFrame;
            if (m_isLogging) {
                std::cout << getTimeStamp(TimeResolution::ms) << " new frame " << m_cntFrame << std::endl;
            }
        }
        m_cndNewFrame.notify_one();


        {
            std::unique_lock<std::mutex> stopLock(m_mtxStop);
            if (m_cndStop.wait_for(stopLock, durationPerFrame, [this]{return m_isReleased;})) {
                if (m_isLogging) {
                    std::cout << getTimeStamp(TimeResolution::ms) << " stop thread at frame " << getFrameCount() << std::endl;
                }
                break;
            } else {
                if (m_isLogging) {
                    std::cout << getTimeStamp(TimeResolution::ms) << " timed out " << getFrameCount() << std::endl;
                }
            }
        }
    }

}


/*
 * read capture properties in same format as cv::VideoCapture
 */
double VideoCaptureSimu::get(int propid) {
    switch (propid) {
    case cv::CAP_PROP_FPS:
        return m_fps;
    case cv::CAP_PROP_FRAME_HEIGHT:
        return m_frameSize.height;
    case cv::CAP_PROP_FRAME_WIDTH:
        return m_frameSize.width;
    default:
        return 0;
    }
}


int VideoCaptureSimu::getFrameCount() {
    return m_cntFrame;
}


bool VideoCaptureSimu::read(cv::Mat& frame) {
    std::unique_lock<std::mutex> newFrameLock(m_mtxNewFrame);
    m_cndNewFrame.wait(newFrameLock, [this]{return m_isNewFrame;} );
    m_sourceFrame.copyTo(frame);
    m_isNewFrame = false;
    if (m_isLogging) {
        std::cout << getTimeStamp(TimeResolution::ms) << " frame read " << getFrameCount() << std::endl;
    }
    return true;
}


void VideoCaptureSimu::release() {
    {
        std::lock_guard<std::mutex> lock(m_mtxStop);
        m_isReleased = true;
    }
    m_cndStop.notify_one();
}


bool VideoCaptureSimu::selectVideoSize(std::string videoSize) {
    for (std::pair<std::string, cv::Size> elem : m_availVideoSizes) {
        if (elem.first.find(videoSize) != std::string::npos) {
            m_frameSize = elem.second;
            std::cout << "video size: " << m_frameSize <<" selected" << std::endl;
            return true;
        }
    }
    std::cout << "video size: " << videoSize <<" not selectable" << std::endl;
    return false;
}


/*
 * write capture properties in same format as cv::VideoCapture
 */
bool VideoCaptureSimu::set(int propid, double value) {
    switch (propid) {
        case cv::CAP_PROP_FPS: {
            const size_t fpsMax = 60;
            const size_t fpsMin = 1;
            size_t fps = static_cast<size_t>(value);
            if (fps > fpsMax) {
                m_fps = fpsMax;
                std::cout << "fps set to max: " << fps << std::endl;
            } else {
                if (fps < 1) {
                    m_fps = fpsMin;
                    std::cout << "fps set to min: " << fps << std::endl;
                } else {
                    m_fps = fps;
                    std::cout << "fps set to: " << fps << std::endl;
                }
            }
            return true;
        }

        case cv::CAP_PROP_FRAME_WIDTH: {
            std::string width = std::to_string(static_cast<int>(value));
            return selectVideoSize(width);
        }

        case cv::CAP_PROP_FRAME_HEIGHT: {
            std::string height = std::to_string(static_cast<int>(value));
            return selectVideoSize(height);
        }

        default:
            return false;
    }
}
