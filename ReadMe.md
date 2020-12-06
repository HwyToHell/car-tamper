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
   main-write: 			test cv::VideoWriter and cv::VideoCapture get & set
   motionbuffer.cpp:	ring buffer class with saveToDisk capability
   save-main: 			test motion detection in separate thread

.
├── car-tamper-test
│   ├── car-tamper-test.pro
│   ├── car-tamper-test.pro.user
│   ├── inc
│   │   └── video-capture-simu.h
│   ├── src
│   │   ├── main-test-cases.cpp
│   │   ├── motionbuffer-test.cpp
│   │   ├── vid-cap-simu-test.cpp
│   │   └── video-capture-simu.cpp
│   └── test-cases.ods
├── FSpec - Car Tamper.odt
├── inc
│   ├── backgroundsubtraction.h
│   ├── motionbuffer.h
│   └── time-stamp.h
├── qtcr-car-tamper
│   ├── qtcr-car-tamper.pro
│   └── qtcr-car-tamper.pro.user
├── ReadMe.md
├── RSpec - Car Tamper.odt
├── src
│   ├── backgroundsubtraction.cpp
│   ├── main-buf.cpp
│   ├── main-cap.cpp
│   ├── main-clock.cpp
│   ├── main-mat.cpp
│   ├── main-qt-file-picker.cpp
│   ├── main-read.cpp
│   ├── main-verify-write.cpp
│   ├── main-write.cpp
│   ├── motionbuffer-bak01.cpp
│   ├── motionbuffer.cpp
│   ├── save-main.cpp
│   └── time-stamp.cpp
└── ToDo.md

