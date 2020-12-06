#include "../inc/motionbuffer.h"
#include "../inc/time-stamp.h"

#define UNIT_TEST


/* LogFrame ******************************************************************/
LogFrame::LogFrame(std::string subDir) {
    std::string logDir = cv::utils::fs::getcwd();

    if (!subDir.empty()) {
        logDir += "/";
        logDir += subDir;
        if (cv::utils::fs::exists(logDir) && cv::utils::fs::isDirectory(logDir)) {
            std::cout << "log dir already exists: " << subDir << std::endl;
            m_logSubDir = subDir + "/";

        // log dir does not exist yet
        } else {
            if (cv::utils::fs::createDirectory(subDir)) {
                std::cout << "log dir created: " << subDir << std::endl;
                m_logSubDir = subDir + "/";
            } else {
                std::cout << "cannot create log dir: " << subDir << std::endl;
                std::cout << "use current directory instead" << std::endl;
            }
        }
    }
}


LogFrame::~LogFrame() {
    close();
}


void LogFrame::close() {
    if (m_logFile.isOpened())
        m_logFile.release();
}


bool LogFrame::create() {
    std::string fileName = m_logSubDir + getTimeStamp(TimeResolution::ms_NoBlank);
    fileName += ".json";
    if (m_logFile.open(fileName, cv::FileStorage::Mode::WRITE)) {
        std::cout << "log file created: " << fileName << std::endl;
    } else {
        std::cout << "cannot create log file: " << fileName << std::endl;
    }
    return m_logFile.isOpened();
}


void LogFrame::write(cv::Mat frame) {
    int rows = frame.rows;
    std::string timeStampAsKey = "_" + getTimeStamp(TimeResolution::ms_NoBlank);
    m_logFile << timeStampAsKey;
    m_logFile << "{" << "frame count" << frame.at<int>(0,0);
    m_logFile <<        "time stamp" << frame.at<int>(0,rows - 1) << "}";
}




/* MotionBuffer **************************************************************/
MotionBuffer::MotionBuffer(std::size_t preBufferSize, double fpsOutput, std::string logDirForTest) :
    m_activateSaveToDisk{false},
    m_fps{fpsOutput},
    m_frameSize{cv::Size(640,480)},
    m_isBufferAccessible{false},
    m_isSaveToDiskRunning{false},
    m_logAtTest{logDirForTest},
    m_terminate{false}
{
    /* limit preBufferSize in order to
     * have saveToDisk algo work properly (min: 1)
     * avoid heap memory shortage (max: 60) */
    if (preBufferSize < 1) {
        m_preBufferSize = 1;
    } else if (preBufferSize > 60) {
        m_preBufferSize = 60;
    } else {
        m_preBufferSize = preBufferSize;
    }
    m_thread = std::thread(&MotionBuffer::saveMotionToDisk, this);
}


MotionBuffer::MotionBuffer(std::size_t preBufferSize, double fpsOutput) :
    MotionBuffer{preBufferSize, fpsOutput, ""}
{

}


MotionBuffer::~MotionBuffer() {
    DEBUG(getTimeStampMs() << " destructor called");
    releaseBuffer();
    if (m_thread.joinable()) {
        m_thread.join();
        DEBUG(getTimeStampMs() << " thread joined");
    }
    assert(!m_thread.joinable());
    DEBUG(getTimeStampMs() << " destructor finished");
}


void MotionBuffer::activateSaveToDisk(bool value) {
    m_activateSaveToDisk = value;
}


void MotionBuffer::pushToBuffer(cv::Mat& frame) {
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
        m_buffer.push_front(frame.clone());
        m_isBufferAccessible = true;
        }
        m_cndBufferAccess.notify_one();

    // NOT saveToDiskRunning
    } else {
        // no lock guard needed
        m_buffer.push_front(frame.clone());

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


bool MotionBuffer::popBuffer(cv::Mat& out) {
    if (m_buffer.size() > 0) {
        out = m_buffer.back().clone();
        m_buffer.pop_back();
        return true;
    } else {
        return false;
    }
}


void MotionBuffer::printBuffer() {
    std::deque<cv::Mat>::iterator it = m_buffer.begin();
    int cnt = 0;
    while ( it != m_buffer.end() ) {
        std::string imgFile = getTimeStampMs() + ".jpg";
        cv::imwrite(imgFile, *it);
        ++it;
        std::cout << "frame from buffer #" << ++cnt << std::endl;
    }
}


void MotionBuffer::releaseBuffer() {
    DEBUG(getTimeStampMs() << " release called");
    m_terminate = true;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_activateSaveToDisk = false;
        m_isBufferAccessible = true;
    }
    m_cndBufferAccess.notify_one();
    DEBUG(getTimeStampMs() << " thread notified");
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
        if (m_terminate) return;

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
            #ifdef UNIT_TEST
            m_logAtTest.create();
            #endif
            DEBUG(getTimeStampMs() << " video file created");

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
            #ifdef UNIT_TEST
            m_logAtTest.write(lastFrame);
            #endif
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
            #ifdef UNIT_TEST
            m_logAtTest.close();
            #endif
            DEBUG(getTimeStampMs() << " videoWriter released");
        }


    } // while thread not terminated
}


/* Functions *****************************************************************/
