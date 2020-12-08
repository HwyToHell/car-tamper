# Car Tamper Application
-----------------------

### detect motion on webcam video stream and
### save video sequences to disk
FSpec: feature spec with implementaion details
RSpec: requirements
- car-tamper-test
   cartaper-test.pro: qt creator project for unit testing
   - src
      main-test-cases:		test runner using catch.org
      motion-buffer-test: 	test cases
	  vid-cap-simu-test: 	test cases for video-capture-simu
      video-capture-simu: 	cv::VideoCapture compatible simulator
- inc
- src
   main-cap:			test video capture loop
   main-clock:			test std::chrono
   main-mat:			test cv::Mat
   main-qt-file-picker:	test video capture from file (qt file picker)
   main-read: 			test video capture from file (cmd line arg)
   main-read-json:		test cv::FileNode by reading json file
   main-write: 			test cv::VideoWriter and cv::VideoCapture get & set
   motionbuffer.cpp:	ring buffer class with saveToDisk capability
   save-main: 			test motion detection in separate thread

- __car\-tamper__
   - [FSpec \- Car Tamper.odt](FSpec%20-%20Car%20Tamper.odt)
   - [RSpec \- Car Tamper.odt](RSpec%20-%20Car%20Tamper.odt)
   - [ReadMe.md](ReadMe.md)
   - [ToDo.md](ToDo.md)
   - __car\-tamper\-test__
     - [car\-tamper\-test.pro](car-tamper-test/car-tamper-test.pro)
     - [car\-tamper\-test.pro.user](car-tamper-test/car-tamper-test.pro.user)
     - __inc__
       - [video\-capture\-simu.h](car-tamper-test/inc/video-capture-simu.h)
     - __src__
       - [main\-test\-cases.cpp](car-tamper-test/src/main-test-cases.cpp)
       - [motionbuffer\-test.cpp](car-tamper-test/src/motionbuffer-test.cpp)
       - [vid\-cap\-simu\-test.cpp](car-tamper-test/src/vid-cap-simu-test.cpp)
       - [video\-capture\-simu.cpp](car-tamper-test/src/video-capture-simu.cpp)
     - [test\-cases.ods](car-tamper-test/test-cases.ods)
   - __inc__
     - [backgroundsubtraction.h](inc/backgroundsubtraction.h)
     - [motionbuffer.h](inc/motionbuffer.h)
     - [time\-stamp.h](inc/time-stamp.h)
   - __qtcr\-car\-tamper__
     - [qtcr\-car\-tamper.pro](qtcr-car-tamper/qtcr-car-tamper.pro)
     - [qtcr\-car\-tamper.pro.user](qtcr-car-tamper/qtcr-car-tamper.pro.user)
   - __src__
     - [backgroundsubtraction.cpp](src/backgroundsubtraction.cpp)
     - [main\-cap.cpp](src/main-cap.cpp)
     - [main\-clock.cpp](src/main-clock.cpp)
     - [main\-mat.cpp](src/main-mat.cpp)
     - [main\-qt\-file\-picker.cpp](src/main-qt-file-picker.cpp)
     - [main\-read\-json.cpp](src/main-read-json.cpp)
     - [main\-read.cpp](src/main-read.cpp)
     - [main\-verify\-write.cpp](src/main-verify-write.cpp)
     - [main\-write.cpp](src/main-write.cpp)
     - [motionbuffer\-bak01.cpp](src/motionbuffer-bak01.cpp)
     - [motionbuffer.cpp](src/motionbuffer.cpp)
     - [save\-main.cpp](src/save-main.cpp)
     - [time\-stamp.cpp](src/time-stamp.cpp)

