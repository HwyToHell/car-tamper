#ifndef VIDEOCAPTURESIMU_H
#define VIDEOCAPTURESIMU_H
#include <opencv2/opencv.hpp>

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>


class VideoCaptureSimu
{
public:
    VideoCaptureSimu(size_t framesPerSecond = 10);
    ~VideoCaptureSimu();
    bool    read(cv::Mat& frame);
    bool    set();
    void    stop();
private:
    void generateFrame(size_t fps);
    std::condition_variable m_cndNewFrame;
    std::condition_variable m_cndStop;
    size_t m_fps;
    cv::Size m_frameSize;
    bool m_isNewFrame;
    bool m_isStopped;
    std::mutex m_mtxNewFrame;
    std::mutex m_mtxStop;
    cv::Mat m_sourceFrame;
    std::thread m_thread;
};

#endif // MOTIONBUFFER_H
