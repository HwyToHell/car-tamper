# Car Tamper Application
-----------------------

### TODO
[2020-12-06]
- bakground subtraction
- finish motion buffer: configure output directory and file name


- change dir structure to
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
- optional: delete dir qtcr-car-tamper and move qtcr-car-tamper.pro up one level
- optional: create dir wip for the main-xxx files



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

