#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H
#include "motionbuffer.h"
#include "backgroundsubtraction.h"

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
    /* background subtractor: theshold of frame difference */
    void        bgrSubThreshold(double threshold);
    double      bgrSubThreshold() const;
    void        debugMode(bool debug);
    bool        debugMode() const;
    cv::Mat     debugMotionMask() const;
    bool        hasFrameMotion(cv::Mat frame);
    bool        isContinuousMotion(cv::Mat frame);
    /* duration as number of update steps */
    void        minMotionDuration(int value);
    int         minMotionDuration() const;
    int         motionDuration() const;
    /* area in per cent of frame area */
    void        minMotionIntensity(int value);
    int         minMotionIntensity() const;
    cv::Mat     motionMask() const;
    void        resetBackground();
    /* region of interest related to upper left corner */
    void        roi(cv::Rect);
    cv::Rect    roi() const;

    // TODO reset backgroundsubtractor
private:
    cv::Ptr<BackgroundSubtractorLowPass> m_bgrSub;
    bool        m_debug;
    cv::Mat     m_debugMotionMask;
    bool        m_isContinuousMotion;
    int         m_minMotionDuration;
    int         m_minMotionIntensity;
    int         m_motionDuration;
    int         m_motionIntensity;
    cv::Mat     m_motionMask;
    cv::Mat     m_resizedFrame;
    cv::Mat     m_processedFrame;
    cv::Rect    m_roi;
};

#endif // MOTIONDETECTOR_H
