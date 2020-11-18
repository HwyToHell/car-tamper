#include "../inc/video-capture-simu.h"
#include "../../inc/motionbuffer.h"

#include <cassert>
#include <functional>


VideoCaptureSimu::VideoCaptureSimu(size_t framesPerSecond) :
    m_frameSize{cv::Size(640,480)},
    m_isNewFrame{false},
    m_isStopped{false}
{
    m_thread = std::thread(&VideoCaptureSimu::generateFrame, this, framesPerSecond);
}


VideoCaptureSimu::~VideoCaptureSimu() {
    stop();
    assert(!m_thread.joinable());
}


cv::Mat frameTimeStamp(cv::Size frameSize) {
    const cv::Scalar black  = cv::Scalar(0,0,0);
    const cv::Scalar red    = cv::Scalar(0,0,255);
    cv::Mat frame(frameSize, CV_8UC3, black);
    std::string timeStamp = getTimeStamp();
    cv::putText(frame, timeStamp, cv::Point(10,50),
                cv::FONT_HERSHEY_SIMPLEX, 1, red, 2);

    return frame;
}

void VideoCaptureSimu::generateFrame(size_t fps) {
    size_t msPerFrame = 1000 / fps;
    auto durationPerFrame = std::chrono::milliseconds(msPerFrame);

    std::cout << "frame duration: " << durationPerFrame.count() << std::endl;

    while (!m_isStopped) {
        {
            std::lock_guard<std::mutex> newFrameLock(m_mtxNewFrame);
            m_sourceFrame = frameTimeStamp(m_frameSize);
            m_isNewFrame = true;
            m_cndNewFrame.notify_one();
        }
        std::cout << "generate frame" << std::endl;
        std::this_thread::sleep_for(durationPerFrame);

        /*
        {
            std::unique_lock<std::mutex> stopLock(m_mtxStop);
            m_cndStop.wait_for(stopLock, durationPerFrame, [this]{return m_isStopped;});
        }
        */
    }

}


bool VideoCaptureSimu::read(cv::Mat& frame) {
    std::unique_lock<std::mutex> newFrameLock(m_mtxNewFrame);
    m_cndNewFrame.wait(newFrameLock, [this]{return m_isNewFrame;} );
    m_sourceFrame.copyTo(frame);
    m_isNewFrame = false;
    return true;
}


void VideoCaptureSimu::stop() {
    std::lock_guard<std::mutex> lock(m_mtxStop);
    m_isStopped = true;
    m_cndStop.notify_one();
    m_thread.join();
}
