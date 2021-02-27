#include "../inc/motionbuffer.h"



/* LogFrame ******************************************************************/
LogFrame::LogFrame(std::string subDir)
{
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


LogFrame::~LogFrame()
{
    close();
}


void LogFrame::close()
{
    if (m_logFile.isOpened())
        m_logFile.release();
}


bool LogFrame::create(std::string fileName)
{

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


std::string LogFrame::getLogFileRelPath()
{
    return (m_logSubDir + m_logFileName);
}


void LogFrame::write(cv::Mat frame)
{
    int rows = frame.rows;
    std::string timeStampAsKey = "_" + getTimeStamp(TimeResolution::micSec_NoBlank);
    m_logFile << timeStampAsKey;
    m_logFile << "{" << "frame count" << frame.at<int>(0,0);
    m_logFile <<        "time stamp" << frame.at<int>(0,rows - 1) << "}";
}




/* MotionBuffer **************************************************************/
MotionBuffer::MotionBuffer(std::size_t preBufferSize,
                           double      fps,
                           std::string videoDir,
                           std::string logDir,
                           bool        logging,
                           bool        timeFromFile) :
    m_frameCount{0},
    /* current frame size set in pushToBuffer */
    m_frameSize{cv::Size(0,0)},
    m_isBufferAccessible{false},
    m_isLogging{logging},
    m_isNewFile{false},
    m_isSaveToDiskRunning{false},
    m_isTimeFromFile{timeFromFile},
    m_logAtTest{logDir},
    /* keep postBuffer the same size of preBuffer, potential source of error */
    m_postBufferSize{1},
    m_preBufferSize{1},
    m_saveToDiskState{State::createVideoFile},
    m_setSaveToDisk{false},
    m_terminate{false}
{
    preBuffer(preBufferSize);
    postBuffer(preBufferSize);
    fpsOutput(fps);
    setVideoDir(videoDir);

    m_threadSaveToDisk = std::thread(&MotionBuffer::saveMotionToDisk, this);
}


MotionBuffer::~MotionBuffer()
{
    DEBUG(getTimeStampMs() << " " << __func__ << ", started, #" << __LINE__);
    // release buffer and terminate thread saveToDisk;
    releaseBuffer();

    m_setSaveToDisk = false;
    m_terminate = true;
    m_isBufferAccessible = true;
    m_cndBufferAccess.notify_one();
    DEBUG(getTimeStampMs() << " " << __func__ << ", thread saveToDisk notified, #" << __LINE__);

    if (m_threadSaveToDisk.joinable()) {
        m_threadSaveToDisk.join();
        DEBUG(getTimeStampMs() << " " << __func__ << ", thread joined, #" << __LINE__);
    }
    assert(!m_threadSaveToDisk.joinable());
    DEBUG(getTimeStampMs() << " " << __func__ << ", finished, #" << __LINE__);
}


void MotionBuffer::fpsOutput(double fps)
{
    /* limit frames per second for output video in order to
     * observe reasonable motion (min: 1)
     * avoid processor ressource shortage (max: 60) */
    m_fps = fps;
    if (m_fps < 1) {
        m_fps = 1;
    } else if (m_fps > 60) {
        m_fps = 60;
    }
}


double MotionBuffer::fpsOutput() const
{
    return m_fps;
}


std::string MotionBuffer::getLogFileRelPath()
{
    return m_logAtTest.getLogFileRelPath();
}


std::string MotionBuffer::getVideoFileName()
{
    std::string videoFileName;

    if (isSaveToDiskRunning()) {
        DEBUG(getTimeStampMs() << " " << __func__ << ", save to disk running ... waiting for video file, #" << __LINE__);
        videoFileName = waitForVideoFile();
        DEBUG(getTimeStampMs() << " " << __func__ << ", video file: " << videoFileName << ", #" << __LINE__);
    } else {
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            videoFileName = m_videoFileName;
        }
        DEBUG(getTimeStampMs() << " " << __func__ << ", video file: " << videoFileName << ", #" << __LINE__);
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
bool MotionBuffer::isPostBufferFinished()
{
    cv::Mat lastFrame;
    // DEBUG(getTimeStampMs() << " bufSize " << m_buffer.size());
    // TAM-32 fixed
    bool manyFramesAvailabe = m_buffer.size() > 1;

    while (manyFramesAvailabe && m_remainingPostFrames) {
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.back().copyTo(lastFrame);
            m_buffer.pop_back();
            manyFramesAvailabe = m_buffer.size() > 1;
        }
        DEBUG(getTimeStampMs() << " " << __func__ << ", post frame " << m_remainingPostFrames << " copied, #" << __LINE__);

        m_videoWriter.write(lastFrame);
        if (m_isLogging) {
            m_logAtTest.write(lastFrame);
        }
        DEBUG(getTimeStampMs() << " " << __func__ << ", post frame " << m_remainingPostFrames << " written, #" << __LINE__);

        --m_remainingPostFrames;

    } /* while write frames until buffer emptied */
    return (m_remainingPostFrames == 0);
}


void MotionBuffer::preBuffer(std::size_t nFrames)
{
    /* limit preBufferSize in order to have saveToDisk algo work properly (min: 2)
     * avoid heap memory shortage (max: 60) */
    m_preBufferSize = nFrames;
    if (m_preBufferSize < 2) {
        m_preBufferSize = 2;
        std::cout << "pre buffer too small, set to minimum size of "
                  << m_preBufferSize << std::endl;
    } else if (m_preBufferSize > 60) {
        m_preBufferSize = 60;
        std::cout << "pre buffer too large, set to minimum size of "
                  << m_preBufferSize << std::endl;
    }
}


size_t MotionBuffer::preBuffer() const
{
    return m_preBufferSize;
}


void MotionBuffer::postBuffer(size_t nFrames)
{
    /* limit postBufferSize to same values as preBuffer */
    m_postBufferSize = nFrames;
    if (m_postBufferSize < 2) {
        m_postBufferSize = 2;
        std::cout << "post buffer too small, set to minimum size of "
                  << m_postBufferSize << std::endl;
    } else if (m_postBufferSize > 60) {
        m_postBufferSize = 60;
        std::cout << "post buffer too large, set to minimum size of "
                  << m_postBufferSize << std::endl;
    }
}


size_t MotionBuffer::postBuffer() const
{
    return m_postBufferSize;
}


void MotionBuffer::pushToBuffer(cv::Mat& frame)
{
    bool saveToDiskRunning = false;
    bool setSaveToDisk = false;
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        saveToDiskRunning = m_isSaveToDiskRunning;
        setSaveToDisk = m_setSaveToDisk;
        ++m_frameCount;
    }

    if (saveToDiskRunning) {
        /* lock guard needed because of deque size change */
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.push_front(frame.clone());
            m_isBufferAccessible = true;
        }
        m_cndBufferAccess.notify_one();
        DEBUG(getTimeStampMs() << " " << __func__ << ", newFrameToBuffer notified, #" << __LINE__);

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
            DEBUG(getTimeStampMs() << " " << __func__ << ", save to disk activated, #" << __LINE__);
            // TAM-33 fixed
            m_frameSize = frame.size();
            m_isSaveToDiskRunning = true;
            m_isBufferAccessible = true;
            m_cndBufferAccess.notify_one();
        }
    }
}


void MotionBuffer::releaseBuffer()
{
    DEBUG(getTimeStampMs() << " " << __func__ << ", started, #" << __LINE__);

    /* move to state create in order to close videoWriter */
    toStateCreate();
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_setSaveToDisk = false;
        m_frameCount = 0;
    }
}


void MotionBuffer::resetNewMotionFile()
{
    m_isNewFile = false;
}


/* run thread func until terminate signal has been received */
void MotionBuffer::saveMotionToDisk()
{
    DEBUG(getTimeStampMs() << " " << __func__ << ", wait for newFrameToBuffer started, #" << __LINE__);

    while (!m_terminate) {

        /* wait for new frame pushed to buffer */
        {
            std::unique_lock<std::mutex> bufferLock(m_mtxBufferAccess);
            m_cndBufferAccess.wait(bufferLock, [this]{return m_isBufferAccessible;});
            m_isBufferAccessible = false;
        }
        std::stringstream ss;
        ss << "bufferSize: " << m_buffer.size()
           << ", setSaveToDisk: " << m_setSaveToDisk
           << ", isSaveToDiskRunning: " << m_isSaveToDiskRunning;
        DEBUG(getTimeStampMs() << " " << __func__ << ", wait for newFrameToBuffer finished,\n" << ss.str() << " #" << __LINE__);

        if (m_terminate) {
            DEBUG(getTimeStampMs() << " " << __func__ << ", terminate thread, #" << __LINE__);
            return;
        }

        switch (m_saveToDiskState) {
        case State::noMotion:
            break;
        /* open new video file for writing */
        case State::createVideoFile:
        {
            m_videoFileName = timeStamp() + ".avi";
            std::string fileNameRel = m_videoSubDir + m_videoFileName;
            int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4');
            assert(m_frameSize != cv::Size(0,0));

            if(!m_videoWriter.open(fileNameRel, fourcc, m_fps, m_frameSize)) {
                std::cout << "cannot open file: " << fileNameRel << std::endl;
                m_isSaveToDiskRunning = false;
                m_terminate = true;
                break;
            }
            if (m_isLogging) {
                m_logAtTest.create(getTimeStamp(TimeResolution::ms_NoBlank) + ".json");
            }
            DEBUG(getTimeStampMs() << " " << __func__ << ", video file created, #" << __LINE__);

            m_saveToDiskState = State::writeActiveMotion;
            break;
        }

        case State::writeActiveMotion:
            writeUntilBufferEmpty();

            if (m_saveToDiskState == State::writePostBuffer) {
                if (isPostBufferFinished()) {
                    DEBUG(getTimeStampMs() << " " << __func__ << ", all post frames written, #" << __LINE__);
                    // release videoWriter
                    toStateCreate();
                } else {
                    DEBUG(getTimeStampMs() << " " << __func__ << ", buffer empty, not able to write all post frames, #" << __LINE__);
                }
            }
            break;

        case State::writePostBuffer:
            if (isPostBufferFinished()) {
                DEBUG(getTimeStampMs() << " " << __func__ << ", all post frames written, #" << __LINE__);
                // release videoWriter
                toStateCreate();
            }
            break;

        } /* switch state */
    } /* while thread not terminated */
}


void MotionBuffer::setSaveToDisk(bool value)
{
    // on rising and falling edge
    if (m_setSaveToDisk != value) {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_setSaveToDisk = value;
        DEBUG(getTimeStampMs() << " " << __func__ << ", setSaveToDisk: " << m_setSaveToDisk << ", #" << __LINE__);
    }
}


void MotionBuffer::startTime(std::tm time)
{
    // tm -> time_t, time_t -> chrono
    // add milliseconds in timeStamp
    time_t epochTime = mktime(&time);
    m_startTime = std::chrono::system_clock::from_time_t(epochTime);

    // reset frame counter
    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_frameCount = 0;
    }
}


time_t MotionBuffer::startTime() const
{
    return std::chrono::system_clock::to_time_t(m_startTime);
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


std::string MotionBuffer::timeStamp()
{
    // from file: add offset to start time of video file
    if (m_isTimeFromFile) {
        int offsetMs = 0;
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            offsetMs = static_cast<int>(m_frameCount * 1000 / m_fps);
        }
        auto offset = std::chrono::milliseconds(offsetMs);
        auto currentTimePoint = m_startTime + offset;
        return getTimeStamp(TimeResolution::sec_NoBlank, currentTimePoint);
    }
    // from cam: use current time
    else {
        return getTimeStamp(TimeResolution::sec_NoBlank);
    }
}


void MotionBuffer::toStateCreate() {
    DEBUG(getTimeStampMs() << " " << __func__ << ", post buffer finished, releasing videoWriter, #" << __LINE__);
    m_videoWriter.release();
    m_saveToDiskState = State::createVideoFile;
    if (m_isLogging) {
        m_logAtTest.close();
    }
    DEBUG(getTimeStampMs() << " " << __func__ << ", videoWriter released, #" << __LINE__);

    {
        std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
        m_isSaveToDiskRunning = false;
        m_isBufferAccessible = false;
    }

    /* notify caller thread waitForVideoFile(),
     * that there is a new video file available */
    m_isNewFile = true;
    m_cndNewFile.notify_one();
    DEBUG(getTimeStampMs() << " " << __func__ << ", thread waitForVideoFile notified, #" << __LINE__);
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
void MotionBuffer::writeUntilBufferEmpty()
{
    cv::Mat lastFrame;
    bool manyFramesAvailabe = true;
    bool stopWriting = false;
    size_t bufferSize = 0;
    while (manyFramesAvailabe) {
        {
            std::lock_guard<std::mutex> bufferLock(m_mtxBufferAccess);
            m_buffer.back().copyTo(lastFrame);
            m_buffer.pop_back();
            bufferSize = m_buffer.size();
            manyFramesAvailabe = bufferSize > 1;
            stopWriting = !m_setSaveToDisk;
        }
        DEBUG(getTimeStampMs() << " " << __func__ << ", bufSize: " << m_buffer.size() << ", frame copied, #" << __LINE__);

        m_videoWriter.write(lastFrame);
        if (m_isLogging) {
            m_logAtTest.write(lastFrame);
        }
       DEBUG(getTimeStampMs() << " " << __func__ << ", frame written, #" << __LINE__);

        if (stopWriting) {
            m_saveToDiskState = State::writePostBuffer;
            m_remainingPostFrames = m_postBufferSize + bufferSize;
            std::stringstream ss;
            ss << "bufSize: " << bufferSize;
            DEBUG(getTimeStampMs() << " " << __func__ << ss.str() << ", start writing post buffer, #" << __LINE__);
            break;
        }

    } /* while write frames until buffer emptied */
}
