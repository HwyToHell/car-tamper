#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H
#include "backgroundsubtraction.h"
#include "motionbuffer.h"

#include <opencv2/opencv.hpp>

enum class MotionMinimal {intensity, duration};
enum class Duration {frames, seconds};

// TODO
// setMotionArea - in per cent of frame area
// setMotionDuration - in sec, in number of frames (alternative)
// setMotionRoi - in cv::Rect

class MotionDetector
{
public:
    MotionDetector();
    bool        enableSaveToDisk(MotionBuffer& buffer);
    cv::Mat     motionMask();
    bool        hasFrameMotion(cv::Mat frame);

    /* area in per cent of frame area */
    void        minMotionIntensity(int value);
    int         minMotionIntensity();

    /* duration as number of update steps */
    void        minMotionDuration(int value);
    int         minMotionDuration();
    void        resetBackground();

    /* region of interest related to upper left corner */
    void        roi(cv::Rect);
    cv::Rect    roi();
    int         updateMotionDuration(bool isMotion);

    // TODO get and set
    // foreRun, overRun (Vor- und Nachlauf in Anzahl Frames)
    // reset backgroundsubtractor
private:
    cv::Ptr<BackgroundSubtractorLowPass> m_bgrSub;
    bool        m_isSaveToDiskEnabled;
    int         m_minMotionDuration;
    int         m_minMotionIntensity;
    int         m_motionDuration;
    cv::Mat     m_motionMask;
    cv::Rect    m_roi;

};

#endif // MOTIONDETECTOR_H
