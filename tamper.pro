TEMPLATE = app
TARGET = tamper
#QT -= gui
QT += core gui widgets

CONFIG += c++14 console
#CONFIG += debug
CONFIG -= app_bundle




DEFINES -= DEBUG_BUILD
DEFINES -= LOG_AT_TEST


#CONFIG(debug, debug|release):DEFINES += DEBUG_BUILD
#CONFIG(debug, debug|release):message(Debug build)
#CONFIG(release, debug|release):message(Release build)
#message("defines: "$$DEFINES)


linux {
LIBS += -lopencv_core \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_video \
        -lopencv_videoio
LIBS += -lv4l2 -lv4l1
LIBS += -lstdc++fs
    contains(QT_ARCH, "arm") {
        message("ARM cross")
        INCLUDEPATH += /home/holger/app-dev/pi4-qt/sysroot/usr/local/include
    }
}

windows {
DEFINES += _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
INCLUDEPATH += $$(OPENCV_DIR)/include
    CONFIG(debug,debug|release) {
    message(debug build!)
    LIBS += -L$$(OPENCV_DIR)/x64/vc16/lib \
    -lopencv_core346d \
    -lopencv_highgui346d \
    -lopencv_imgcodecs346d \
    -lopencv_imgproc346d \
    -lopencv_video346d \
    -lopencv_videoio346d
    }
    CONFIG(release,debug|release) {
    message(release build!)
    LIBS += -L$$(OPENCV_DIR)/x64/vc16/lib \
    -lopencv_core346 \
    -lopencv_highgui346 \
    -lopencv_imgcodecs346 \
    -lopencv_imgproc346 \
    -lopencv_video346 \
    -lopencv_videoio346
    }
}

message($$LIBS)
#message($$(OPENCV_DIR))



HEADERS += \
    ../cpp/inc/rlutil.h \
    inc/backgroundsubtraction.h \
    inc/motion-detector.h \
    inc/motionbuffer.h \
    inc/time-stamp.h \
    inc/video-capture-simu.h


SOURCES += \
    src/backgroundsubtraction.cpp \
    src/cli-analyzer.cpp \
    src/motion-detector.cpp \
    src/motionbuffer.cpp \
    src/time-stamp.cpp \
    src/video-capture-simu.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /home/pi
!isEmpty(target.path): INSTALLS += target
