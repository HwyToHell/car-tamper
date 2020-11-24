#include "../inc/motionbuffer.h"


MotionBuffer::MotionBuffer(std::size_t preBufferSize) :
    m_activateSaveToDisk{false},
    m_fps{10},
    m_frameSize{cv::Size(640,480)},
    m_isBufferAccessible{false},
    m_isSaveToDiskRunning{false},
    m_preBufferSize{preBufferSize},
    m_terminate{false}
{
    m_thread = std::thread(&MotionBuffer::saveMotionToDisk, this);
}


MotionBuffer::~MotionBuffer() {
    stopBuffer();
    assert(!m_thread.joinable());
}


void MotionBuffer::pushFrameToBuffer(cv::Mat& frame) {
    bool saveToDiskRunning = false;
    {   std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        saveToDiskRunning = m_isSaveToDiskRunning;
    }
    DEBUG(getTimeStampMs() << " frame pushed to buffer"
          << ", saveToDiskRunning: " << saveToDiskRunning);

    if (saveToDiskRunning) {
        // lock guard needed because of deque size change
        {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_buffer.push_front(frame);
        m_isBufferAccessible = true;
        }
        m_cndBufferAccess.notify_one();

    // NOT saveToDiskRunning
    } else {
        // no lock guard needed
        m_buffer.push_front(frame);

        // delete last frame if ring buffer size exeeded
        if (m_buffer.size() > m_preBufferSize) {
            m_buffer.pop_back();
        }
        DEBUG("buffer size: " << m_buffer.size());

        // notify thread that -> will set saveToDiskRunning
        if (m_activateSaveToDisk) {
            std::cout << "save to disk activated" << std::endl;
            m_isBufferAccessible = true;
            m_cndBufferAccess.notify_one();
        }
    }
}


void MotionBuffer::saveMotionToDisk() {
    // run thread func until terminate signal is received
    cv::VideoWriter videoWriter;
    cv::Mat lastFrame;
    DEBUG(getTimeStampMs() << " wait for newFrameToBuffer started");

    while (!m_terminate) {

        // wait for new frame pushed to buffer
        {   std::unique_lock<std::mutex> bufferLock(m_mtxBufferAccess);
            m_cndBufferAccess.wait(bufferLock, [this]{return m_isBufferAccessible;});
        }
        DEBUG(getTimeStampMs() << " wait for newFrameToBuffer finished");
        if (m_terminate) break;

        // open new video file for writing
        bool startWriting = false;
        {   std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            startWriting = m_activateSaveToDisk && !m_isSaveToDiskRunning;
        }
        if (startWriting) {
            std::string filename = getTimeStamp(TimeResolution::sec) + ".avi";
            int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4');

            if(!videoWriter.open(filename, fourcc, m_fps, m_frameSize)) {
                std::cout << "cannot open file: " << filename << std::endl;
                m_isSaveToDiskRunning = false;
                m_terminate = true;
                break;
            }
            DEBUG(getTimeStampMs() << "video file created");
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_isSaveToDiskRunning = true;
        }

        // continue writing frames until buffer emptied
        bool manyFramesAvailabe = true;
        while (manyFramesAvailabe) {
            {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.back().copyTo(lastFrame);
            m_buffer.pop_back();
            manyFramesAvailabe = m_buffer.size() > 1;
            }
            DEBUG(getTimeStampMs() << " last frame copied");

            videoWriter.write(lastFrame);
            DEBUG(getTimeStampMs() << " last frame written");

        } // while write frames until buffer emptied

        bool stopWriting = false;
        {   std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_isBufferAccessible = false;
            stopWriting = !m_activateSaveToDisk;
        }
        if (stopWriting) {
            m_isSaveToDiskRunning = false;
            videoWriter.release();
            DEBUG(getTimeStampMs() << " videoWriter released");
        }


    } // while thread not terminated
}


void MotionBuffer::stopBuffer() {
    std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
    m_activateSaveToDisk = false;
    m_isBufferAccessible = true;
    m_cndBufferAccess.notify_one();

}


void MotionBuffer::activateSaveToDisk(bool value) {
    m_activateSaveToDisk = value;
}


std::string getTimeStamp(TimeResolution resolution) {
    // time stamp in std::chrono format
    auto nowTimePoint = std::chrono::system_clock::now();
    auto nowMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>
            (nowTimePoint.time_since_epoch());
    auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(nowMilliSec);

    // convert seconds to time_t for pretty printing with put_time
    std::time_t nowSecTimeT = nowSec.count();

    std::stringstream timeStamp;
    timeStamp << std::put_time(std::localtime(&nowSecTimeT), "%F %T");
    if (resolution == TimeResolution::ms) {
        long nowMilliSecRemainder = nowMilliSec.count() % 1000;
        timeStamp << "." << nowMilliSecRemainder;
    }

    return timeStamp.str();
}
