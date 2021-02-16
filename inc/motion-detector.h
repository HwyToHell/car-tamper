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
    cv::Mat     motionMask();
    bool        hasFrameMotion(cv::Mat frame);
    bool        isContinuousMotion(cv::Mat frame);

    /* area in per cent of frame area */
    void        minMotionIntensity(int value);
    int         minMotionIntensity();

    /* duration as number of update steps */
    void        minMotionDuration(int value);
    int         minMotionDuration();
    int         motionDuration();
    void        resetBackground();

    /* region of interest related to upper left corner */
    void        roi(cv::Rect);
    cv::Rect    roi();

    // TODO
    // reset backgroundsubtractor
private:
    cv::Mat     clipFrame(cv::Mat frame, cv::Rect roi);
    cv::Ptr<BackgroundSubtractorLowPass> m_bgrSub;
    bool        m_isContinuousMotion;
    int         m_minMotionDuration;
    int         m_minMotionIntensity;
    int         m_motionDuration;
    cv::Mat     m_motionMask;
    cv::Rect    m_roi;

};

#endif // MOTIONDETECTOR_H
