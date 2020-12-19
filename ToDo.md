# Car Tamper Application
-----------------------

### TODO

[2020-12-19]
- test motionbuffer in combination with motion-detector 
- motion-detector get / set
- OK: video-capture-simu: implement motion simulation
  with number of motion pixels as parameter


[2020-12-10]
motionbuffer module
- get log file name
- get / set abs path to Video (and Log File)


[2020-12-06]
- background subtraction
- OK: finish motion buffer: configure output directory and file name


- OK: change dir structure to
	inc
	src
	test (from car-tamper-test)
- car-tamper-test
	inc, src -> remove
	move vid-capture-simu up one level (.h and .cpp)
- test
	only contains:
		main-test-cases.cpp
		module-name-test.cpp
		test-cases.ods
		test.pro
- OK: rename test project "car-tamper-test" -> "test-car-tamper"
  easier to recognize when opening project with qt-creator
- OK: optional: delete dir qtcr-car-tamper and move qtcr-car-tamper.pro up one level
- OK: optional: create dir wip for the main-xxx files
- OK: motionbuffer.cpp: rename activateSaveToDisk -> setSaveToDisk




[2020-12-05]
- OK: vid-cap-simu: encode time stamp in first pixel of last row

[2020-11-26]
MotionBuffer
- OK: test other frame rates (30 fps)
- OK: test larger pre-buffer size
- OK: avoid memory overflow for too large buffer sizes 
      done by limiting preBufferSize to 60 and smaller

[2020-11-14]
- OK: log overflow in motion detection thread
- OK: MVP for thread_save_video
- describe test cases

