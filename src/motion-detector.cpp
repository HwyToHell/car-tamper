#include "../inc/motion-detector.h"

MotionDetector::MotionDetector() :
    m_isSaveToDiskEnabled{false},
    m_minMotionDuration{10},    // number of consecutive frames with motion
    m_minMotionIntensitiy{100}  // number of pixels with motion
{
    m_bgrSub = createBackgroundSubtractorLowPass(0.005, 40);

}


bool MotionDetector::enableSaveToDisk(MotionBuffer& buffer)
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


int MotionDetector::get(MotionMinimal parameter)
{
    int value = 0;
    switch (parameter) {
    case MotionMinimal::intensity:
        value = m_minMotionIntensitiy;
        break;
    case MotionMinimal::duration:
        value = m_minMotionDuration;
        break;
    }
    return value;
}


cv::Mat MotionDetector::getMotionFrame()
{
    return m_motionMask;
}


bool MotionDetector::hasFrameMotion(cv::Mat frame)
{
    // pre-processing
    cv::Mat processedFrame;
    cv::blur(frame, processedFrame, cv::Size(10,10));

    // detect motion in current frame
    m_bgrSub->apply(processedFrame, m_motionMask);
    int motionIntensity = cv::countNonZero(m_motionMask);
    return motionIntensity > m_minMotionIntensitiy ? true : false;
}


bool MotionDetector::set(MotionMinimal parameter, int value)
{
    // boundary validation of input 0 ... 100
    value = value > 100 ? 100 : value;
    value = value < 0 ? 0 : value;

    switch (parameter) {
    case MotionMinimal::intensity:
        m_minMotionIntensitiy = value;
        break;
    case MotionMinimal::duration:
        m_minMotionDuration = value;
        break;
    }

    return true;
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
