# Car Tamper Application
-----------------------
 
## detect motion on webcam video stream and
## save video sequences to disk

### Branches
- analyze-hd-rpi
   - video source: HD webcam (HQCAM)
   - determine settings for real time analysis on raspberry pi
- win-logicam
   - video source: USB Cam (Logitech 640x480) connected to raspberry pi
   - shell script on raspberry pi generates 1 h chunks of continuous video footage
   - powershell script for windows pc fetches video from raspberry pi and does the motion analysis
   - by invoking the cpp command line analyzer

- __qmake DEFINES__ 
   - DEBUG_BUILD enables debug messages to console
   - LOG_AT_TEST log video-capture-simu encoded information about frame timing

### Project structure

- __tamper__
   - [performance\-tamper.txt](performance-tamper.txt) performance measurement comparison between PC and Raspberry 
   - __inc__
     - [backgroundsubtraction.h](inc/backgroundsubtraction.h)
     - [motionbuffer.h](inc/motionbuffer.h)     
     - [motion\-detector.h](inc/motion-detector.h)
     - [time\-stamp.h](inc/time-stamp.h)
     - [video\-capture\-simu.h](inc/video-capture-simu.h)
   - __ps__
     - powershell scripts for fetching video files from rpi, local motion analysis and file management
   - __res__
     - ressource files for different purposes
   - __sh__
     - shell scripts for raspberry pi (e.g. delete old video files to avoid memory shortage on SD card)
   - __spec__
     - [confluence-tamper-spec.sh](spec/confluence-tamper-spec.sh)          link to confluence spec (RSpec and FSpec have been migrated)
     - [FSpec \- Car Tamper.odt](spec/FSpec%20-%20Car%20Tamper.odt)			feature spec with implementaion details
     - [RSpec \- Car Tamper.odt](spec/RSpec%20-%20Car%20Tamper.odt)			requirements
   - __src__
     - [backgroundsubtraction.cpp](src/backgroundsubtraction.cpp)		 	class backgroundsubtraction
     - [cli\-analyzer.cpp](src/cli-analyzer.cpp) main module for command line analyzer
     - [motion\-detector.cpp](src/motion-detector.cpp)						class for motion detection
     - [main\-flicker.cpp](src/main-flicker.cpp)                            test opencv filter functions to reduce flicker
     - [motionbuffer.cpp](src/motionbuffer.cpp)								class ringbuffer with saveToDisk capability
     - [time\-stamp.cpp](src/time-stamp.cpp)								functions for time stamp generation
     - [video\-capture\-simu.cpp](src/video-capture-simu.cpp)				class for compatible cv::VideoCapture simulation
      - __wip__																mains for testing purposes
         - [main\-analyze\-video.cpp](src/main-analyze-video.cpp)				test motion detection by analyzing video file
         - [main\-cap\-get\-set.cpp](src/wip/main-cap-get-set.cpp)			test cv::VideoWriter and cv::VideoCapture get & set
         - [main\-cap.cpp](src/wip/main-cap.cpp)							test video capture loop
         - [main\-clock.cpp](src/wip/main-clock.cpp)						test std::chrono
         - [main\-compose\-path.cpp](src/main-compose-path.cpp) test opencv::util::filesystem functionality for path handling
         - [main\-detect\-motion.cpp](src/main-detect-motion.cpp)			test motion detection by using video capture simu
         - [main\-detect\-motion\-cam.cpp](src/main-detect-motion-cam.cpp) test motion detection by processing real camera input
         - [main\-detect\-motion\-file.cpp](src/main-detect-motion-file.cpp) test motion detection by processing video file
         - [main\-progress.cpp](src/main-progress.cpp) test progress indicator on command line
         - [main\-mat.cpp](src/wip/main-mat.cpp)							test cv::Mat
         - [main\-qt\-file\-picker.cpp](src/wip/main-qt-file-picker.cpp)	test video capture from file (qt file picker)
         - [main\-read.cpp](src/wip/main-read.cpp)							test video capture from file (cmd line arg), initial MVP (Nov 21)
         - [main\-read\-json.cpp](src/wip/main-read-json.cpp)               test cv::FileStorage functionality by reading json file
         - [main\-rtsp\-stream.cpp](src/main-rtsp-stream.cpp) test reading rtsp stream from ip-cam
         - [main\-show\-video.cpp](src/main-show-video.cpp) test program options with downsized application, skipping analyzing step, just showing  input video   
         - [main\-time\-from\-filename.cpp](src/main-time-from-filename.cpp) determine recording time from file name
         - [main\-video\-writer.cpp](src/main-video-writer.cpp) test cv::VideoCapture from cam, get and set parameters, cv::VideoWriter in different formats
         - [main\-write\-to\-disk.cpp](src/main-write-to-disk.cpp)			test write to disk functionality
   - __test__																test cases
     - [main\-test\-cases.cpp](test/main-test-cases.cpp)					test runner using catch.org
     - [motionbuffer\-test.cpp](test/motionbuffer-test.cpp)					test cases motionbuffer
     - [test\-cases.ods](test/test-cases.ods)								test case description
     - [test\-tamper.pro](test/test-tamper.pro)								qtcr project for unit testing
     - [vid\-cap\-simu\-test.cpp](test/vid-cap-simu-test.cpp)				test cases video-capture-simu
