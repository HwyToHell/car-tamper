#include "../inc/motion-detector.h"

MotionDetector::MotionDetector() :
    m_isContinuousMotion{false},
    m_minMotionDuration{10},    // number of consecutive frames with motion
    m_minMotionIntensity{100},  // number of pixels with motion
    m_motionDuration{0}
{
    // default -> alpha: 0.005 threshold: 40
    m_bgrSub = createBackgroundSubtractorLowPass(0.005, 50);

}


// clipFrame
cv::Mat clipFrame(cv::Mat frame, cv::Rect roi)
{
    return frame(roi);
}


bool MotionDetector::hasFrameMotion(cv::Mat frame)
{
    // pre-processing of clipped frame
    cv::Mat processedFrame;
    cv::blur(frame(m_roi), processedFrame, cv::Size(10,10));

    // detect motion in current frame
    m_bgrSub->apply(processedFrame, m_motionMask);
    int motionIntensity = cv::countNonZero(m_motionMask);
    bool isMotion = motionIntensity > m_minMotionIntensity ? true : false;

    // update motion duration
    // motion increase by 1
    if (isMotion) {
        ++m_motionDuration;
        m_motionDuration = m_motionDuration > m_minMotionDuration
                ? m_minMotionDuration : m_motionDuration;

    // no motion decrease by 1
    } else {
        --m_motionDuration;
        m_motionDuration = m_motionDuration <= 0
                ? 0 : m_motionDuration;
    }

    return isMotion;
}


bool MotionDetector::isContinuousMotion(cv::Mat frame)
{
    hasFrameMotion(frame);

    if (m_motionDuration >= m_minMotionDuration) {
        m_isContinuousMotion = true;
    } else if (m_motionDuration == 0) {
        m_isContinuousMotion = false;
    }

    return m_isContinuousMotion;
}


void MotionDetector::minMotionDuration(int value)
{
    /* allow 300 update steps at max */
    value = value > 300 ? 300 : value;
    value = value < 0 ? 0 : value;
    m_minMotionDuration = value;
}


int MotionDetector::minMotionDuration()
{
    return m_minMotionDuration;
}


void MotionDetector::minMotionIntensity(int value)
{
    /* per cent of frame area */
    value = value > 100 ? 100 : value;
    value = value < 0 ? 0 : value;
    m_minMotionIntensity = value;
}


int MotionDetector::minMotionIntensity()
{
    return m_minMotionIntensity;
}


int MotionDetector::motionDuration()
{
    return m_motionDuration;
}


cv::Mat MotionDetector::motionMask()
{
    return m_motionMask;
}


void MotionDetector::roi(cv::Rect value)
{
    m_roi = value;
}


cv::Rect MotionDetector::roi()
{
    return m_roi;
}
