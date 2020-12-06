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

