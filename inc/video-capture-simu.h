#ifndef VIDEOCAPTURESIMU_H
#define VIDEOCAPTURESIMU_H
#include <opencv2/opencv.hpp>

#include <chrono>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <thread>

enum class GenMode {black, timeStamp, motionArea, motionAreaAndTime};


class VideoCaptureSimu
{
public:
    VideoCaptureSimu(size_t framesPerSecond = 10, std::string videoSize = "640x480");
    ~VideoCaptureSimu();
    double      get(int propid);
    int         getFrameCount();
    bool        read(cv::Mat& frame);
    bool        set(int propid, double value);
    bool        setMode(GenMode mode, int area = 0, int greyLevel = 0);
    bool        setTimeStampMode();
    void        release();
private:
    void        generateFrame();
    bool        selectVideoSize(std::string videoSize);
    const std::map<std::string, cv::Size>   m_availVideoSizes;
    std::condition_variable                 m_cndNewFrame;
    std::condition_variable                 m_cndStop;
    int                                     m_cntFrame;
    size_t                                  m_fps;
    cv::Size                                m_frameSize;
    GenMode                                 m_genMode;
    bool                                    m_isLogging;
    bool                                    m_isNewFrame;
    bool                                    m_isReleased;
    int                                     m_motionArea;
    int                                     m_motionGreyLevel;
    std::mutex                              m_mtxNewFrame;
    std::mutex                              m_mtxStop;
    cv::Mat                                 m_sourceFrame;
    std::thread                             m_thread;
};

#endif // VIDEOCAPTURESIMU_H
