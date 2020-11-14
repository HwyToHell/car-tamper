#include "../inc/motionbuffer.h"

std::string getTimeStamp() {
    // time stamp in std::chrono format
    auto nowTimePoint = std::chrono::system_clock::now();
    auto nowMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>
            (nowTimePoint.time_since_epoch());
    auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(nowMilliSec);

    // convert seconds to time_t for pretty printing with put_time
    std::time_t nowSecTimeT = nowSec.count();
    long nowMilliSecRemainder = nowMilliSec.count() % 1000;

    std::stringstream timeStamp;
    timeStamp << std::put_time(std::localtime(&nowSecTimeT), "%F %T")
              << "." << nowMilliSecRemainder;
    return timeStamp.str();
}

MotionBuffer::MotionBuffer(std::size_t preBufferSize, int detectionSampleRatio) :
    m_detectionSampleRatio{detectionSampleRatio},
    m_isFrameForDetectionReady{false},
    m_isMotionDetected{false},
    m_preBufferSize{preBufferSize}
{

}


cv::Mat MotionBuffer::getFrameForDetection() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cndFrameForDetectionReady.wait(lock, [this]{return m_isFrameForDetectionReady;});
    //m_cndFrameForDetectionReady.wait(lock);

    // reset frame ready
    m_isFrameForDetectionReady = false;

    // return copy of frame in order to ensure thread safety
    return m_buffer.front().clone();
}


cv::Mat MotionBuffer::getMotionFrame() {
    // runs on thread_save_video
    return cv::Mat();
}


void MotionBuffer::setMotionDetected(bool isMotionDetected) {
    // isSaveToDiskReady (= es sind noch frames zu schreiben) && isMotionDetected == true
    // -> set
    m_isMotionDetected = isMotionDetected;
}


void MotionBuffer::stopDetection() {
    std::lock_guard<std::mutex> lock(m_mtx);
    m_isFrameForDetectionReady = true;
    m_cndFrameForDetectionReady.notify_one();
}


void MotionBuffer::writeFrameToBuffer(cv::Mat& frame) {
    // must be locked for thread safety
    std::lock_guard<std::mutex> lock(m_mtx);
    m_buffer.push_front(frame);

    // use every n-th frame only for motion detection
    // modulo division continues to work after signed integer overflow
    if ( (++m_frameCount % m_detectionSampleRatio) == 0) {
        std::cout << "frame sent: " << m_frameCount << std::endl;
        std::cout << "ringbuffer size: " << m_buffer.size() << std::endl;

        // thread_motion_detection has not reset condition -> overflow
        if (m_isFrameForDetectionReady) {
            std::cout << "motion detection overflow at " << getTimeStamp() << std::endl;
        }
        m_isFrameForDetectionReady = true;
        m_cndFrameForDetectionReady.notify_one();
    }

    // no motion -> keep preBufferSize constant
    if (!m_isMotionDetected) {
        if (m_buffer.size() > m_preBufferSize) {
            m_buffer.pop_back();
        }
    }

    // signal write access to buffer

}
