#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H
#include "backgroundsubtraction.h"
#include "motionbuffer.h"

#include <opencv2/opencv.hpp>

enum class MotionMinimal {intensity, duration};

class MotionDetector
{
public:
    MotionDetector();
    bool    enableSaveToDisk(MotionBuffer& buffer);
    int     get(MotionMinimal parameter);
    cv::Mat getMotionFrame();
    bool    hasFrameMotion(cv::Mat frame);
    bool    set(MotionMinimal parameter, int value);
    int     updateMotionDuration(bool isMotion);

    // TODO get and set
    // foreRun, overRun (Vor- und Nachlauf in Anzahl Frames)
    // reset backgroundsubtractor
private:
    cv::Ptr<BackgroundSubtractorLowPass> m_bgrSub;
    bool m_isSaveToDiskEnabled;
    int m_minMotionDuration;
    int m_minMotionIntensitiy;
    int m_motionDuration;
    cv::Mat m_motionMask;




};

#endif // MOTIONDETECTOR_H
