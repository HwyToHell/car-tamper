#include "../inc/motion-detector.h"

MotionDetector::MotionDetector() :
    m_isSaveToDiskEnabled{false},
    m_minMotionDuration{10},    // number of consecutive frames with motion
    m_minMotionIntensitiy{100}  // number of pixels with motion
{
    m_bgrSub = createBackgroundSubtractorLowPass(0.005, 40);

}


bool MotionDetector::enableSaveToDisk(MotionBuffer buffer)
{
    if (m_motionDuration >= m_minMotionDuration
            && !buffer.isSaveToDiskRunning()) {
        m_isSaveToDiskEnabled = true;
    } else if (m_motionDuration == 0) {
        m_isSaveToDiskEnabled = false;
    }
    buffer.setSaveToDisk(m_isSaveToDiskEnabled);
    return m_isSaveToDiskEnabled;
}


bool MotionDetector::hasFrameMotion(cv::Mat frame)
{
    cv::Mat postFrame;
    cv::blur(frame, postFrame, cv::Size(10,10));

    // detect motion in current frame
    cv::Mat motionMask;
    m_bgrSub->apply(postFrame, motionMask);
    int motionIntensity = cv::countNonZero(motionMask);
    return motionIntensity > m_minMotionIntensitiy ? true : false;
}


int MotionDetector::updateMotionDuration(bool isMotion) {
    /* limit motion duration */
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

    return m_motionDuration;
}
