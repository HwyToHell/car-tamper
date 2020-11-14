#ifndef MOTIONBUFFER_H
#define MOTIONBUFFER_H
#include <opencv2/opencv.hpp>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>


class MotionBuffer
{
public:
    MotionBuffer(std::size_t preBufferSize = 30, int detectionSampleRatio = 3);
    cv::Mat getFrameForDetection();
    cv::Mat getMotionFrame();
    void    setMotionDetected(bool isMotionDetected);
    void    stopDetection();
    void    writeFrameToBuffer(cv::Mat& frame);
private:
    bool isFrameForDetectionReady();
    std::deque<cv::Mat> m_buffer;
    std::condition_variable m_cndFrameForDetectionReady;
    int m_detectionSampleRatio;
    int m_frameCount;
    bool m_isFrameForDetectionReady;
    bool m_isMotionDetected;
    std::mutex m_mtx;
    std::size_t m_preBufferSize;


};

#endif // MOTIONBUFFER_H
