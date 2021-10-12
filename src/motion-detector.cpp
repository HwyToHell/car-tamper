#include "../inc/motion-detector.h"

MotionDetector::MotionDetector() :
    m_debug{false},
    m_isContinuousMotion{false},
    m_minMotionDuration{10},    // number of consecutive frames with motion
    m_minMotionIntensity{100},  // number of pixels with motion
    m_motionDuration{0},
    m_roi{0,0,0,0}
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


void MotionDetector::debugMode(bool debug)
{
    m_debug = debug;
}


bool MotionDetector::debugMode() const
{
    return m_debug;
}


cv::Mat MotionDetector::debugMotionMask() const
{
    return m_debugMotionMask;
}


bool MotionDetector::hasFrameMotion(cv::Mat frame)
{
    // use full frame size, if roi is not initialized yet
    if (m_roi == cv::Rect(0,0,0,0))
        m_roi = cv::Rect(cv::Point(0,0), frame.size());

    /* pre-processing of clipped frame */
    // TODO implement clipping cv::blur(frame(m_roi), processedFrame, cv::Size(10,10));

    // intermediate step: resize full HD frame 1920x1080 -> 480x270
    cv::resize(frame, m_resizedFrame, cv::Size(), 0.25, 0.25, cv::INTER_LINEAR);
    int kernel = m_resizedFrame.size().width / 96;
    cv::blur(m_resizedFrame, m_processedFrame, cv::Size(kernel,kernel));
    //m_processedFrame = m_resizedFrame;

    // detect motion in current frame
    m_bgrSub->apply(m_processedFrame, m_motionMask);
    m_motionIntensity = cv::countNonZero(m_motionMask);
    bool isMotion = m_motionIntensity > m_minMotionIntensity ? true : false;

    if (m_debug) {
        // std::cout << "intensitiy: " << m_motionIntensity << "  " << isMotion << std::endl;
        // std::cout << "m_motionMask channels: " << m_motionMask.channels() << " size: " << m_motionMask.size << std::endl;

        /* color motionMask based on intensity */
        int width = m_processedFrame.size().width;
        int height = m_processedFrame.size().height;
        cv::Mat whiteBgr(height, width, CV_8UC3, cv::Scalar(255,255,255));
        cv::Mat yellowBgr(height, width, CV_8UC3, cv::Scalar(0,255,255));
        cv::Mat orangeBgr(height, width, CV_8UC3, cv::Scalar(0,127,255));
        cv::Mat redBgr(height, width, CV_8UC3, cv::Scalar(0,0,255));
        cv::Mat coloredMask(height, width, CV_8UC3, cv::Scalar(0,0,0));
        if (isMotion) {
            if (m_isContinuousMotion) {
                // motion mask in red
                cv::bitwise_and(redBgr, redBgr, coloredMask, m_motionMask);
            } else {
                // motion mask in yellow
                cv::bitwise_and(yellowBgr, yellowBgr, coloredMask, m_motionMask);
            }
        } else {
            if (m_isContinuousMotion) {
                // motion mask in orange
                cv::bitwise_and(orangeBgr, yellowBgr, coloredMask, m_motionMask);
            } else {
                // motion mask in white
                cv::bitwise_and(whiteBgr, whiteBgr, coloredMask, m_motionMask);
            }
        }
        m_debugMotionMask = coloredMask;
        // cv::imshow("resized frame", m_resizedFrame);
        // cv::imshow("colored mask", coloredMask);
    }   // DEBUG_END

    // update motion duration
    // motion -> increase by 1
    if (isMotion) {
        ++m_motionDuration;
        m_motionDuration = m_motionDuration > m_minMotionDuration
                ? m_minMotionDuration : m_motionDuration;

    // no motion -> decrease by 1
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
    //* per cent of frame area */
    //value = value > 100 ? 100 : value;
    // value = value < 0 ? 0 : value;
    /* number of pixels */
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
