#include "../inc/video-capture-simu.h"
#include "../../inc/motionbuffer.h"

#include <cassert>
#include <functional>
#include <sstream>


VideoCaptureSimu::VideoCaptureSimu(size_t framesPerSecond) :
    m_availVideoSizes{
        {"320x240", cv::Size(320,240)},
        {"640x480", cv::Size(640,480)},
        {"1024x768", cv::Size(1024,768)}
    },
    m_cntFrame{0},
    m_fps{framesPerSecond},
    m_frameSize{m_availVideoSizes.find("640x480")->second},
    m_isLogging{false},
    m_isNewFrame{false},
    m_isReleased{false}

{
    m_thread = std::thread(&VideoCaptureSimu::generateFrame, this, framesPerSecond);
}


VideoCaptureSimu::~VideoCaptureSimu() {
    release();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    assert(!m_thread.joinable());
}


cv::Mat frameTimeStamp(cv::Size frameSize) {
    const cv::Scalar black  = cv::Scalar(0,0,0);
    const cv::Scalar red    = cv::Scalar(0,0,255);
    cv::Mat frame(frameSize, CV_8UC3, black);
    std::string timeStamp = getTimeStamp(TimeResolution::ms);
    cv::putText(frame, timeStamp, cv::Point(10,50),
                cv::FONT_HERSHEY_SIMPLEX, 1, red, 2);

    return frame;
}

void VideoCaptureSimu::generateFrame(size_t fps) {
    size_t msPerFrame = 1000 / fps;
    auto durationPerFrame = std::chrono::milliseconds(msPerFrame);
    if (m_isLogging) {
        std::cout << "frame duration: " << durationPerFrame.count() << std::endl;
    }

    while (!m_isReleased) {
        {
            std::lock_guard<std::mutex> newFrameLock(m_mtxNewFrame);
            m_sourceFrame = frameTimeStamp(m_frameSize);
            m_isNewFrame = true;
            ++m_cntFrame;
            if (m_isLogging) {
                std::cout << getTimeStamp(TimeResolution::ms) << " new frame " << m_cntFrame << std::endl;
            }
        }
        m_cndNewFrame.notify_one();
        // std::this_thread::sleep_for(durationPerFrame);

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


std::string VideoCaptureSimu::get(VidCapProps param) {
    std::string ret = "";
    switch (param) {
    case VidCapProps::fps: {
        ret = std::to_string(m_fps);
        break;
        }
    case VidCapProps::frameSize: {
        std::stringstream ss;
        ss << m_frameSize;
        ret = ss.str();
        break;
        }
    case VidCapProps::isLogging: {
        ret = std::to_string(m_isLogging);
        break;
        }
    }
    return ret;
}

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


bool VideoCaptureSimu::set(VidCapProps param, std::string value) {
    bool ret = true;
    switch (param) {
    case VidCapProps::isLogging: {
        m_isLogging = std::stoul(value) > 0 ? true : false;
        break;
        }
    case VidCapProps::fps: {
        size_t fps = std::stoul(value);
        m_fps = ((fps > 100 ? 100 : fps) < 1 ? 1 : fps);
        break;
        }
    case VidCapProps::frameSize: {
        if (m_availVideoSizes.find(value) != m_availVideoSizes.end()) {
            m_frameSize = m_availVideoSizes.at(value);
        } else {
            std::cout << "video size: " << value <<" not known" << std::endl;
            ret = false;
        }
        break;
        }
    }
    return ret;
}
