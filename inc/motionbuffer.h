#ifndef MOTIONBUFFER_H
#define MOTIONBUFFER_H
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>
#include <opencv2/core/persistence.hpp>

// #include <chrono>
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

/*
 * helper class for unit testing of MotionBuffer
 * writes frame count and frame time stamp to file
 */
class LogFrame {
public:
    LogFrame(std::string subDir = "");
    ~LogFrame();
    void close();
    bool create();
    void write(cv::Mat frame);
private:
    std::string m_logSubDir;
    cv::FileStorage m_logFile;
};


/*
 * stores frames in ring buffer
 * after saveToDisk mode has been enabled:
 * write buffer and consecutive frames to file
 * until saveToDisk has been disabled
 */
class MotionBuffer
{
public:
    MotionBuffer(std::size_t preBufferSize = 30);
    ~MotionBuffer();
    void        activateSaveToDisk(bool value);
    bool        popBuffer(cv::Mat& out);
    void        printBuffer();
    void        pushFrameToBuffer(cv::Mat& frame);
    void        releaseBuffer();
    LogFrame    logFrameTest;
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


#endif // MOTIONBUFFER_H
