#ifndef MOTIONBUFFER_H
#define MOTIONBUFFER_H
#include <opencv2/opencv.hpp>

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>


struct Logger {
    bool overflow;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> taskBegin;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> taskEnd;
    std::chrono::milliseconds taskTimeElapsed;
};

class MotionBuffer
{
public:
    MotionBuffer(std::size_t preBufferSize = 30, int detectionSampleRatio = 3);
    cv::Mat getFrameForDetection();
    cv::Mat getMotionFrame();
    void    setMotionDetected(bool isMotionDetected);
    void    setDetectionDone();
    void    stopDetection();
    void    writeFrameToBuffer(cv::Mat& frame);
    Logger  detectionLogger;
private:
    bool isFrameForDetectionReady();
    std::deque<cv::Mat> m_buffer;
    std::condition_variable m_cndFrameForDetectionReady;
    int m_detectionSampleRatio;
    int m_frameCount;
    bool m_isDetectionRunning;
    bool m_isFrameForDetectionReady;
    bool m_isMotionDetected;
    std::mutex m_mtxDetection;
    std::mutex m_mtxSaveVideo;
    std::size_t m_preBufferSize;
};

std::string getTimeStamp();

#endif // MOTIONBUFFER_H
