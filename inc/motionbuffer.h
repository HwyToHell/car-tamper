#ifndef MOTIONBUFFER_H
#define MOTIONBUFFER_H
#include <opencv2/opencv.hpp>

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

#define DEBUG_BUILD

#ifdef DEBUG_BUILD
    #define DEBUG(x) do { std::cerr << x << std::endl; } while (0)
#else
    #define DEBUG(x) do {} while (0)
#endif


struct Logger {
    bool overflow;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> taskBegin;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> taskEnd;
    std::chrono::milliseconds taskTimeElapsed;
};


class MotionBuffer
{
public:
    MotionBuffer(std::size_t preBufferSize = 30);
    ~MotionBuffer();
    void    pushFrameToBuffer(cv::Mat& frame);
    void    stopBuffer();
    void    activateSaveToDisk(bool value);
    Logger  detectionLogger;
private:
    void                    saveMotionToDisk();
    bool                    m_activateSaveToDisk;
    std::deque<cv::Mat>     m_buffer;
    std::condition_variable m_cndBufferAccess;
    double                  m_fps;
    int                     m_frameCount;
    cv::Size                m_frameSize;
    bool                    m_isBufferAccessible;
    bool                    m_isSaveToDiskRunning;
    std::mutex              m_mtxBufferAccess;
    const std::size_t       m_preBufferSize;
    bool                    m_terminate;
    std::thread             m_thread;
};


enum class TimeResolution {ms, sec};


std::string getTimeStamp(TimeResolution resolution);

inline std::string getTimeStampMs() {
    return getTimeStamp(TimeResolution::ms);
};

#endif // MOTIONBUFFER_H
