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
    void        close();
    /* use time stamp as file name, if no file name has been specified */
    bool        create(std::string fileName = "");
    std::string getLogFileRelPath();
    void        write(cv::Mat frame);
private:
    cv::FileStorage m_logFile;
    std::string     m_logFileName;
    std::string     m_logSubDir;

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
    MotionBuffer(std::size_t preBufferSize, double fpsOutput,
                 std::string videoDir       = "videos",
                 std::string logDirForTest  = "log");
    ~MotionBuffer();
    std::string getLogFileRelPath();
    std::string getVideoFileName();
    bool        isSaveToDiskRunning();
    void        pushToBuffer(cv::Mat& frame);
    void        releaseBuffer();
    void        resetNewMotionFile();
    void        setFpsOutput(double fps);
    void        setPostBuffer(std::size_t nFrames);
    void        setPreBuffer(std::size_t nFrames);
    void        setSaveToDisk(bool value);
    bool        setVideoDir(std::string subDir);
    enum        State {
                    noMotion,
                    createVideoFile,
                    writeActiveMotion,
                    writePostBuffer
                };
    std::string waitForVideoFile();
private:
    void                    saveMotionToDisk();
    void                    writeUntilBufferEmpty(cv::VideoWriter& vidWriter);
    bool                    postBufferFinished(cv::VideoWriter& vidWriter);
    bool                    m_setSaveToDisk;
    std::deque<cv::Mat>     m_buffer;
    std::condition_variable m_cndBufferAccess;
    std::condition_variable m_cndNewFile;
    /* frames per second for output video */
    double                  m_fps;
    int                     m_frameCount;
    cv::Size                m_frameSize;
    bool                    m_isBufferAccessible;
    bool                    m_isNewFile;
    bool                    m_isSaveToDiskRunning;
    /* logger for frame count and timinge, used in unit test,
     * enable with #define UNIT_TEST */
    LogFrame                m_logAtTest;
    std::mutex              m_mtxBufferAccess;
    std::mutex              m_mtxNewFileNotice;
    /* number of frames written after saveToDisk has been disabled */
    std::size_t             m_postBufferSize;
    /* preBufferSize must be at least 1 for saveToDisk algo to work
     * and is limited to 60 in order to avoid heap memory shortage */
    std::size_t             m_preBufferSize;
    std::size_t             m_remainingPostFrames;
    State                   m_saveToDiskState;
    bool                    m_terminate;
    std::thread             m_threadSaveToDisk;
    std::string             m_videoSubDir;
    std::string             m_videoFileName;
};


#endif // MOTIONBUFFER_H
