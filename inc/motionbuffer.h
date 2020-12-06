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
    MotionBuffer(std::size_t preBufferSize, double fpsOutput);
    MotionBuffer(std::size_t preBufferSize, double fpsOutput, std::string logDirForTest);
    ~MotionBuffer();
    void        activateSaveToDisk(bool value);
    bool        popBuffer(cv::Mat& out);
    void        printBuffer();
    void        pushToBuffer(cv::Mat& frame);
    void        releaseBuffer();
private:
    void                    saveMotionToDisk();
    bool                    m_activateSaveToDisk;
    std::deque<cv::Mat>     m_buffer;
    std::condition_variable m_cndBufferAccess;
    /* frames per second for output video */
    double                  m_fps;
    int                     m_frameCount;
    cv::Size                m_frameSize;
    bool                    m_isBufferAccessible;
    bool                    m_isSaveToDiskRunning;
    /* logger for frame count and timinge, used in unit test,
     * enable with #define UNIT_TEST */
    LogFrame                m_logAtTest;
    std::mutex              m_mtxBufferAccess;
    /* preBufferSize must be at least 1 for saveToDisk algo to work
     * and is limited to 60 in order to avoid heap memory shortage */
    std::size_t             m_preBufferSize;
    bool                    m_terminate;
    std::thread             m_thread;
};


#endif // MOTIONBUFFER_H
