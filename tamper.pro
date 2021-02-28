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



LIBS += -lopencv_core \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_video \
        -lopencv_videoio
LIBS += -lv4l2 -lv4l1
LIBS += -lstdc++fs

HEADERS += \
    inc/backgroundsubtraction.h \
    inc/motion-detector.h \
    inc/motionbuffer.h \
    inc/time-stamp.h \
    inc/video-capture-simu.h


SOURCES += \
    src/backgroundsubtraction.cpp \
    src/cli-analyzer.cpp \
    src/main-detect-motion-cam.cpp \
    src/main-detect-motion-file.cpp \
    src/main-time-from-filename.cpp \
    src/motion-detector.cpp \
    src/motionbuffer.cpp \
    src/time-stamp.cpp \
    src/video-capture-simu.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
