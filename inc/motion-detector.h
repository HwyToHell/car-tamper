#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H
#include "backgroundsubtraction.h"
#include "motionbuffer.h"

#include <opencv2/opencv.hpp>

class MotionDetector
{
public:
    MotionDetector();

    bool hasFrameMotion(cv::Mat frame);
    bool enableSaveToDisk(MotionBuffer buffer);
    int updateMotionDuration(bool isMotion);

    // TODO get and set
    // motionIntensity, motionDuration
    // reset backgroundsubtractor
private:
    cv::Ptr<BackgroundSubtractorLowPass> m_bgrSub;
    bool m_isSaveToDiskEnabled;
    int m_minMotionDuration;
    int m_minMotionIntensitiy;
    int m_motionDuration;




};

#endif // MOTIONDETECTOR_H
