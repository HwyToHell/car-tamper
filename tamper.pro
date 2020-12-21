TEMPLATE = app
TARGET = tamper
#QT -= gui
QT += core gui widgets

CONFIG += c++14 console
CONFIG -= app_bundle

DEFINES -= DEBUG_BUILD
DEFINES -= LOG_AT_TEST

LIBS += -lopencv_core \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_video \
        -lopencv_videoio
LIBS += -lv4l2 -lv4l1

HEADERS += \
    inc/backgroundsubtraction.h \
    inc/motion-detector.h \
    inc/motionbuffer.h \
    inc/time-stamp.h \
    inc/video-capture-simu.h


SOURCES += \
    src/backgroundsubtraction.cpp \
    src/main-analyze-video.cpp \
    src/main-detect-motion.cpp \
    src/main-write-to-disk.cpp \
    src/motion-detector.cpp \
    src/motionbuffer.cpp \
    src/time-stamp.cpp \
    src/video-capture-simu.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
