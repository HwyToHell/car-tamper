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

        /* log dir does not exist yet */
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


bool LogFrame::create(std::string fileName) {

    if (fileName.empty()) {
        fileName = getTimeStamp(TimeResolution::ms_NoBlank);
        fileName += ".json";
    }
    fileName = m_logSubDir + fileName;

    if (m_logFile.open(fileName, cv::FileStorage::Mode::WRITE)) {
        std::cout << "log file created: " << fileName << std::endl;
        m_logFileName = fileName;
    } else {
        std::cout << "cannot create log file: " << fileName << std::endl;
    }
    return m_logFile.isOpened();
}


std::string LogFrame::getLogFileRelPath() {
    return m_logFileName;
}


void LogFrame::write(cv::Mat frame) {
    int rows = frame.rows;
    std::string timeStampAsKey = "_" + getTimeStamp(TimeResolution::micSec_NoBlank);
    m_logFile << timeStampAsKey;
    m_logFile << "{" << "frame count" << frame.at<int>(0,0);
    m_logFile <<        "time stamp" << frame.at<int>(0,rows - 1) << "}";
}




/* MotionBuffer **************************************************************/
MotionBuffer::MotionBuffer(std::size_t preBufferSize, double fpsOutput,
                           std::string videoDir, std::string logDirForTest) :
    m_setSaveToDisk{false},
    m_fps{fpsOutput},
    m_frameSize{cv::Size(640,480)},
    m_isBufferAccessible{false},
    m_isNewFile{false},
    m_isSaveToDiskRunning{false},
    m_logAtTest{logDirForTest},
    m_preBufferSize{preBufferSize},
    m_terminate{false}
{
    /* limit preBufferSize in order to have saveToDisk algo work properly (min: 1)
     * avoid heap memory shortage (max: 60) */
    if (preBufferSize < 1) {
        m_preBufferSize = 1;
    } else if (preBufferSize > 60) {
        m_preBufferSize = 60;
    }

    /* limit frames per second for output video in order to
     * observe reasonable motion (min: 1)
     * avoid processor ressource shortage (max: 60) */
    if (fpsOutput < 1) {
        m_fps = 1;
    } else if (fpsOutput > 60) {
        m_fps = 60;
    }

    setVideoDir(videoDir);

    m_threadSaveToDisk = std::thread(&MotionBuffer::saveMotionToDisk, this);
}


MotionBuffer::MotionBuffer(std::size_t preBufferSize, double fpsOutput, std::string videoDir) :
    MotionBuffer{preBufferSize, fpsOutput, videoDir, ""}
{

}


MotionBuffer::~MotionBuffer() {
    DEBUG(getTimeStampMs() << " destructor called");
    releaseBuffer();
    if (m_threadSaveToDisk.joinable()) {
        m_threadSaveToDisk.join();
        DEBUG(getTimeStampMs() << " thread joined");
    }
    assert(!m_threadSaveToDisk.joinable());
    DEBUG(getTimeStampMs() << " destructor finished");
}


std::string MotionBuffer::getLogFileRelPath() {
    return m_logAtTest.getLogFileRelPath();
}


std::string MotionBuffer::getVideoFileName() {
    std::string videoFileName;

    if (isSaveToDiskRunning()) {
        DEBUG(getTimeStampMs() << " save to disk running ... waiting for video file");
        videoFileName = waitForMotionFile();
        DEBUG(getTimeStampMs() << " video file availabe");
    } else {
        DEBUG(getTimeStampMs() << " save to disk finished, reading video file name");
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            videoFileName = m_motionFileName;
        }
    }
    return videoFileName;
}


bool MotionBuffer::isSaveToDiskRunning() {
    bool isSaveToDiskRunning;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        isSaveToDiskRunning = m_isSaveToDiskRunning;
    }
    return isSaveToDiskRunning;
}


void MotionBuffer::pushToBuffer(cv::Mat& frame) {
    bool saveToDiskRunning = false;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        saveToDiskRunning = m_isSaveToDiskRunning;
    }
    DEBUG(getTimeStampMs() << " frame pushed to buffer"
          << ", saveToDiskRunning: " << saveToDiskRunning);
    DEBUG("buffer size: " << m_buffer.size());

    if (saveToDiskRunning) {
        /* lock guard needed because of deque size change */
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.push_front(frame.clone());
            m_isBufferAccessible = true;
        }
        m_cndBufferAccess.notify_one();

    /* NOT saveToDiskRunning */
    } else {
        // no lock guard needed
        m_buffer.push_front(frame.clone());

        /* delete last frame if ring buffer size exeeded */
        if (m_buffer.size() > m_preBufferSize) {
            m_buffer.pop_back();
        }

        /* notify thread to start saveToDisk
         * -> will set saveToDiskRunning in other thread */
        if (m_setSaveToDisk) {
            std::cout << "save to disk activated" << std::endl;
            m_isBufferAccessible = true;
            m_cndBufferAccess.notify_one();
        }
    }
}


void MotionBuffer::releaseBuffer() {
    DEBUG(getTimeStampMs() << " release called");

    m_terminate = true;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_setSaveToDisk = false;
        m_isBufferAccessible = true;
    }
    m_cndBufferAccess.notify_one();
    DEBUG(getTimeStampMs() << " thread saveToDisk notified");

    /* thread that calls waitForMotionFile() must be notified,
     * so that this function can finish */
    {
        std::lock_guard<std::mutex> newFileLock(m_mtxNewFileNotice);
        m_isNewFile = true;
    }
    m_cndNewFile.notify_one();
    DEBUG(getTimeStampMs() << " thread waitForMotionFile notified");

}


void MotionBuffer::resetNewMotionFile() {
    m_isNewFile = false;
}


void MotionBuffer::saveMotionToDisk() {
    /* run thread func until terminate signal has been received */
    cv::VideoWriter videoWriter;
    cv::Mat lastFrame;
    DEBUG(getTimeStampMs() << " wait for newFrameToBuffer started");

    while (!m_terminate) {

        /* wait for new frame pushed to buffer */
        {   std::unique_lock<std::mutex> bufferLock(m_mtxBufferAccess);
            m_cndBufferAccess.wait(bufferLock, [this]{return m_isBufferAccessible;});
        }
        DEBUG(getTimeStampMs() << " wait for newFrameToBuffer finished");
        if (m_terminate) return;

        // open new video file for writing
        bool startWriting = false;
        {   std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            startWriting = !m_isSaveToDiskRunning;
            // TODO possible extension:
            //      check, if m_isNewFile has been reset to ensere proper fetching of file name
            //      in module LocalToCloud
        }
        if (startWriting) {
            std::string timeStamp = getTimeStamp(TimeResolution::sec_NoBlank);
            m_motionFileName = timeStamp + ".avi";
            m_videoFilePath = m_videoDirAbs + m_motionFileName;
            int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4');

            if(!videoWriter.open(m_videoFilePath, fourcc, m_fps, m_frameSize)) {
                std::cout << "cannot open file: " << m_videoFilePath << std::endl;
                m_isSaveToDiskRunning = false;
                m_terminate = true;
                break;
            }
            #ifdef UNIT_TEST
            m_logAtTest.create(getTimeStamp(TimeResolution::ms_NoBlank) + ".json");
            #endif
            DEBUG(getTimeStampMs() << " video file created");

            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_isSaveToDiskRunning = true;
        }

        /* continue writing frames until buffer emptied */
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

        } /* while write frames until buffer emptied */

        bool stopWriting = false;
        {   std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_isBufferAccessible = false;
            stopWriting = !m_setSaveToDisk;
        }
        if (stopWriting) {
            m_isSaveToDiskRunning = false;
            DEBUG(getTimeStampMs() << " releasing videoWriter ...");
            videoWriter.release();
            #ifdef UNIT_TEST
            m_logAtTest.close();
            #endif
            DEBUG(getTimeStampMs() << " videoWriter released");

            /* notify other thread that calls waitForMotionFile(),
             * that there is a new video file available */
            m_isNewFile = true;
            m_cndNewFile.notify_one();
        }


    } /* while thread not terminated */
}


void MotionBuffer::setSaveToDisk(bool value) {
    m_setSaveToDisk = value;
}


bool MotionBuffer::setVideoDir(std::string subDir) {
    std::string videoDirAbs = cv::utils::fs::getcwd();

    if (!subDir.empty()) {
        videoDirAbs += "/";
        videoDirAbs += subDir;
        if (cv::utils::fs::exists(videoDirAbs) && cv::utils::fs::isDirectory(videoDirAbs)) {
            std::cout << "directory already exists: " << subDir << std::endl;
            m_videoDirAbs += "/";

        /* dir does not exist yet */
        } else {
            if (cv::utils::fs::createDirectory(subDir)) {
                std::cout << "directory created: " << subDir << std::endl;
                m_videoDirAbs = subDir + "/";
            } else {
                std::cout << "cannot create directory: " << subDir << std::endl;
                return false;
            }
        }
    }
    return true;
}


/* to be called from either main thread (test) or localTocloud thread (production) */
std::string MotionBuffer::waitForMotionFile() {
    std::string videoFileName;
    {
        std::unique_lock<std::mutex> newFileLock(m_mtxNewFileNotice);
        m_cndNewFile.wait(newFileLock, [this]{return m_isNewFile;});
        videoFileName = m_motionFileName;
        m_isNewFile = false;
    }
    return videoFileName;
}



/* Functions *****************************************************************/

