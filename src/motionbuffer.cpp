#include "../inc/motionbuffer.h"
#include "../inc/time-stamp.h"



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
    std::string fileNameRel = m_logSubDir + fileName;

    if (m_logFile.open(fileNameRel, cv::FileStorage::Mode::WRITE)) {
        std::cout << "log file created: " << fileNameRel << std::endl;
        m_logFileName = fileName;
    } else {
        std::cout << "cannot create log file: " << fileNameRel << std::endl;
    }
    return m_logFile.isOpened();
}


std::string LogFrame::getLogFileRelPath() {
    return (m_logSubDir + m_logFileName);
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
                           std::string videoDir,
                           std::string logDir,
                           bool logging) :
    m_setSaveToDisk{false},
    m_fps{fpsOutput},
    m_frameSize{cv::Size(640,480)},
    m_isBufferAccessible{false},
    m_isLogging{logging},
    m_isNewFile{false},
    m_isSaveToDiskRunning{false},
    m_logAtTest{logDir},
    m_postBufferSize{preBufferSize},
    m_preBufferSize{preBufferSize},
    m_saveToDiskState{State::createVideoFile},
    m_terminate{false}
{
    /* limit pre and postBufferSize in order to have saveToDisk algo work properly (min: 1)
     * avoid heap memory shortage (max: 60) */
    if (preBufferSize < 1) {
        m_preBufferSize = 1;
    } else if (preBufferSize > 60) {
        m_preBufferSize = 60;
    }
    m_postBufferSize = m_preBufferSize;

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


MotionBuffer::~MotionBuffer()
{
    DEBUG(getTimeStampMs() << " destructor called");
    releaseBuffer();
    if (m_threadSaveToDisk.joinable()) {
        m_threadSaveToDisk.join();
        DEBUG(getTimeStampMs() << " thread joined");
    }
    assert(!m_threadSaveToDisk.joinable());
    DEBUG(getTimeStampMs() << " destructor finished");
}


std::string MotionBuffer::getLogFileRelPath()
{
    return m_logAtTest.getLogFileRelPath();
}


std::string MotionBuffer::getVideoFileName()
{
    std::string videoFileName;

    if (isSaveToDiskRunning()) {
        DEBUG(getTimeStampMs() << " save to disk running ... waiting for video file");
        videoFileName = waitForVideoFile();
        DEBUG(getTimeStampMs() << " video file availabe");
    } else {
        DEBUG(getTimeStampMs() << " save to disk finished, reading video file name");
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            videoFileName = m_videoFileName;
        }
    }
    return videoFileName;
}


bool MotionBuffer::isSaveToDiskRunning()
{
    bool isSaveToDiskRunning;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        isSaveToDiskRunning = m_isSaveToDiskRunning;
    }
    return isSaveToDiskRunning;
}


/* continue writing frames until buffer emptied or no remaining post frame
 * returns true if complete buffer has been written */
bool MotionBuffer::postBufferFinished(cv::VideoWriter& vidWriter)
{
    cv::Mat lastFrame;
    bool manyFramesAvailabe = true;
    while (manyFramesAvailabe && m_remainingPostFrames) {
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.back().copyTo(lastFrame);
            m_buffer.pop_back();
            manyFramesAvailabe = m_buffer.size() > 1;
        }
        DEBUG(getTimeStampMs() << " post frame " << m_remainingPostFrames << " copied");

        vidWriter.write(lastFrame);
        if (m_isLogging) {
            m_logAtTest.write(lastFrame);
        }
        DEBUG(getTimeStampMs() << " post frame " << m_remainingPostFrames << " written");

        --m_remainingPostFrames;

    } /* while write frames until buffer emptied */
    return (m_remainingPostFrames == 0);
}


void MotionBuffer::pushToBuffer(cv::Mat& frame)
{
    bool saveToDiskRunning = false;
    bool setSaveToDisk = false;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        saveToDiskRunning = m_isSaveToDiskRunning;
        setSaveToDisk = m_setSaveToDisk;
    }

    if (saveToDiskRunning) {
        /* lock guard needed because of deque size change */
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.push_front(frame.clone());
            m_isBufferAccessible = true;
        }
        m_cndBufferAccess.notify_one();
        DEBUG(getTimeStampMs() << " newFrameToBuffer notified");

    /* NOT saveToDiskRunning */
    } else {
        // no lock guard needed
        m_buffer.push_front(frame.clone());

        /* delete last frames if ring buffer size exeeded
         * loop necessary as buffer size could increase well above limit
         * in post writing phase */

        while (m_buffer.size() > m_preBufferSize) {
            m_buffer.pop_back();
            // DEBUG(getTimeStampMs() << " popped to size: " << m_buffer.size());
        }

        /* notify caller thread of saveToDisk () */
        if (setSaveToDisk) {
            DEBUG(getTimeStampMs() << " save to disk activated");
            // TODO isSaveToDiskRunning -> true
            m_isSaveToDiskRunning = true;
            m_isBufferAccessible = true;
            m_cndBufferAccess.notify_one();
        }
    }
}


void MotionBuffer::releaseBuffer()
{
    DEBUG(getTimeStampMs() << " release called");

    // wait for writeToPostBuffer() to be finished
    /*
    if (m_saveToDiskState == State::writePostBuffer) {
        DEBUG(getTimeStampMs() << " in state writePostBuffer: wait for newFile event");
        std::unique_lock<std::mutex> newFileLock(m_mtxNewFileNotice);
        m_cndNewFile.wait(newFileLock, [this]{return m_isNewFile;});
        m_isNewFile = false;
        DEBUG(getTimeStampMs() << " in state writePostBuffer: wait for newFile finished");
    }
    */

    m_terminate = true;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_setSaveToDisk = false;
        m_isBufferAccessible = true;
    }
    m_cndBufferAccess.notify_one();
    DEBUG(getTimeStampMs() << " thread saveToDisk notified");

    /* thread that calls waitForVideoFile() must be notified,
     * so that this function can finish */
    {
        std::lock_guard<std::mutex> newFileLock(m_mtxNewFileNotice);
        m_isNewFile = true;
    }
    m_cndNewFile.notify_one();
    DEBUG(getTimeStampMs() << " thread waitForVideoFile notified");

}


void MotionBuffer::resetNewMotionFile()
{
    m_isNewFile = false;
}


/* run thread func until terminate signal has been received */
void MotionBuffer::saveMotionToDisk()
{
    cv::VideoWriter videoWriter;
    DEBUG(getTimeStampMs() << " wait for newFrameToBuffer started");

    while (!m_terminate) {

        /* wait for new frame pushed to buffer */
        {
            std::unique_lock<std::mutex> bufferLock(m_mtxBufferAccess);
            m_cndBufferAccess.wait(bufferLock, [this]{return m_isBufferAccessible;});
            // DEBUG("bufAccessible: " << m_isBufferAccessible << ", s2dRunning: " << m_isSaveToDiskRunning  << ", setS2D: " << m_setSaveToDisk);
            m_isBufferAccessible = false;
        }
        DEBUG(getTimeStampMs() << " wait for newFrameToBuffer finished");
        if (m_terminate) {
            DEBUG(getTimeStampMs() << " terminate thread saveToDisk");
            return;
        }

        switch (m_saveToDiskState) {
        case State::noMotion:
            break;
        /* open new video file for writing */
        case State::createVideoFile:
        {
            m_videoFileName = getTimeStamp(TimeResolution::sec_NoBlank) + ".avi";
            std::string fileNameRel = m_videoSubDir + m_videoFileName;
            int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4');

            if(!videoWriter.open(fileNameRel, fourcc, m_fps, m_frameSize)) {
                std::cout << "cannot open file: " << fileNameRel << std::endl;
                m_isSaveToDiskRunning = false;
                m_terminate = true;
                break;
            }
            if (m_isLogging) {
                m_logAtTest.create(getTimeStamp(TimeResolution::ms_NoBlank) + ".json");
            }
            DEBUG(getTimeStampMs() << " video file created");

            m_saveToDiskState = State::writeActiveMotion;
            break;
        }
        case State::writeActiveMotion:
            writeUntilBufferEmpty(videoWriter);

            if (m_saveToDiskState == State::writePostBuffer) {
                if (postBufferFinished(videoWriter)) {
                    DEBUG("all post frames written");
                } else {
                    DEBUG("buffer empty, was not able to write all post frames");
                }

                DEBUG(getTimeStampMs() << " post buffer finished, releasing videoWriter ...");
                {
                    std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
                    m_isSaveToDiskRunning = false;
                    m_isBufferAccessible = false;
                }

                videoWriter.release();
                m_saveToDiskState = State::createVideoFile;
                if (m_isLogging) {
                    m_logAtTest.close();
                }
                DEBUG(getTimeStampMs() << " videoWriter released");

                /* notify caller thread waitForVideoFile(),
                 * that there is a new video file available */
                m_isNewFile = true;
                m_cndNewFile.notify_one();
            }
            break;

        } /* switch state */
    } /* while thread not terminated */
    DEBUG(getTimeStampMs() << " saveToDisk finished");
}

void MotionBuffer::setSaveToDisk(bool value)
{
    std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
    m_setSaveToDisk = value;
}


bool MotionBuffer::setVideoDir(std::string subDir)
{
    std::string videoDirAbs = cv::utils::fs::getcwd();

    if (!subDir.empty()) {
        videoDirAbs += "/";
        videoDirAbs += subDir;
        if (cv::utils::fs::exists(videoDirAbs) && cv::utils::fs::isDirectory(videoDirAbs)) {
            std::cout << "directory already exists: " << subDir << std::endl;
            m_videoSubDir = subDir + "/";

        /* dir does not exist yet */
        } else {
            if (cv::utils::fs::createDirectory(subDir)) {
                std::cout << "directory created: " << subDir << std::endl;
                m_videoSubDir = subDir + "/";
            } else {
                std::cout << "cannot create directory: " << subDir << std::endl;
                return false;
            }
        }
    }
    return true;
}


/* to be called from either main thread (test) or localTocloud thread (production) */
std::string MotionBuffer::waitForVideoFile()
{
    std::string videoFileName;
    {
        std::unique_lock<std::mutex> newFileLock(m_mtxNewFileNotice);
        m_cndNewFile.wait(newFileLock, [this]{return m_isNewFile;});
        videoFileName = m_videoFileName;
        m_isNewFile = false;
    }
    return videoFileName;
}


/* continue writing frames to file until buffer emptied */
void MotionBuffer::writeUntilBufferEmpty(cv::VideoWriter& vidWriter)
{
    cv::Mat lastFrame;
    bool manyFramesAvailabe = true;
    bool stopWriting = false;
    while (manyFramesAvailabe) {
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.back().copyTo(lastFrame);
            m_buffer.pop_back();
            manyFramesAvailabe = m_buffer.size() > 1;
            stopWriting = !m_setSaveToDisk;
        }
        DEBUG(getTimeStampMs() << " last frame copied");

        vidWriter.write(lastFrame);
        if (m_isLogging) {
            m_logAtTest.write(lastFrame);
        }
        DEBUG(getTimeStampMs() << " last frame written");

        if (stopWriting) {
            m_saveToDiskState = State::writePostBuffer;
            m_remainingPostFrames = m_postBufferSize;
            DEBUG(getTimeStampMs() << " start writing post buffer");
            break;
        }

    } /* while write frames until buffer emptied */
}


/* Functions *****************************************************************/

