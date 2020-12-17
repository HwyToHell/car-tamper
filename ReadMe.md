# Car Tamper Application
-----------------------

### detect motion on webcam video stream and
### save video sequences to disk

- project structure

- __tamper__
   - [ReadMe.md](ReadMe.md)
   - [ToDo.md](ToDo.md)
   - __inc__
     - [backgroundsubtraction.h](inc/backgroundsubtraction.h)
     - [motionbuffer.h](inc/motionbuffer.h)
     - [time\-stamp.h](inc/time-stamp.h)
     - [video\-capture\-simu.h](inc/video-capture-simu.h)
   - __spec__
     - [FSpec \- Car Tamper.odt](spec/FSpec%20-%20Car%20Tamper.odt)			feature spec with implementaion details
     - [RSpec \- Car Tamper.odt](spec/RSpec%20-%20Car%20Tamper.odt)			requirements
   - __src__
     - [backgroundsubtraction.cpp](src/backgroundsubtraction.cpp)		 	class backgroundsubtraction
     - [main\-write\-to\-disk.cpp](src/main-write-to-disk.cpp)				test write to disk functionality
     - [motionbuffer.cpp](src/motionbuffer.cpp)								class ringbuffer with saveToDisk capability
     - [time\-stamp.cpp](src/time-stamp.cpp)								functions for time stamp generation
     - [video\-capture\-simu.cpp](src/video-capture-simu.cpp)				class for compatible cv::VideoCapture simulation
     - __wip__																mains for testing purposes
       - [main\-cap\-get\-set.cpp](src/wip/main-cap-get-set.cpp)			test cv::VideoWriter and cv::VideoCapture get & set
       - [main\-cap.cpp](src/wip/main-cap.cpp)								test video capture loop
       - [main\-clock.cpp](src/wip/main-clock.cpp)							test std::chrono
       - [main\-mat.cpp](src/wip/main-mat.cpp)								test cv::Mat	
       - [main\-qt\-file\-picker.cpp](src/wip/main-qt-file-picker.cpp)		test video capture from file (qt file picker)
       - [main\-read\-json.cpp](src/wip/main-read-json.cpp)					test cv::FileNode by reading json file
       - [main\-read.cpp](src/wip/main-read.cpp)							test video capture from file (cmd line arg)
       - [main\-verify\-write.cpp](src/wip/main-verify-write.cpp)			test writing frames to disk
   - [tamper.pro](tamper.pro)
   - [tamper.pro.user](tamper.pro.user)
   - __test__																test cases
     - [main\-test\-cases.cpp](test/main-test-cases.cpp)					test runner using catch.org
     - [motionbuffer\-test.cpp](test/motionbuffer-test.cpp)					test cases motionbuffer
     - [test\-cases.ods](test/test-cases.ods)								test case description
     - [test\-tamper.pro](test/test-tamper.pro)								qtcr project for unit testing
     - [test\-tamper.pro.user](test/test-tamper.pro.user)
     - [vid\-cap\-simu\-test.cpp](test/vid-cap-simu-test.cpp)				test cases video-capture-simu

