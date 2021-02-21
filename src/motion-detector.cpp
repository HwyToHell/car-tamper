#include "../inc/motion-detector.h"

MotionDetector::MotionDetector() :
    m_isContinuousMotion{false},
    m_minMotionDuration{10},    // number of consecutive frames with motion
    m_minMotionIntensity{100},  // number of pixels with motion
    m_motionDuration{0}
{
    // default -> alpha: 0.005 threshold: 50
    m_bgrSub = createBackgroundSubtractorLowPass(0.005, 50);

}


void MotionDetector::bgrSubThreshold(double threshold)
{
    /* limit between 0 an 100 */
    threshold = threshold > 100 ? 100 : threshold;
    threshold = threshold < 0 ? 0 : threshold;
    m_bgrSub->threshold(threshold);
}


double MotionDetector::bgrSubThreshold() const
{
    return m_bgrSub->threshold();
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


int MotionDetector::minMotionDuration() const
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


int MotionDetector::minMotionIntensity() const
{
    return m_minMotionIntensity;
}


int MotionDetector::motionDuration() const
{
    return m_motionDuration;
}


cv::Mat MotionDetector::motionMask() const
{
    return m_motionMask;
}


void MotionDetector::roi(cv::Rect value)
{
    m_roi = value;
}


cv::Rect MotionDetector::roi() const
{
    return m_roi;
}
