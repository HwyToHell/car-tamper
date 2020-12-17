TEMPLATE = app
TARGET = tamper
#QT -= gui
QT += core gui widgets

CONFIG += c++14 console
CONFIG -= app_bundle

LIBS += -lopencv_core \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_video \
        -lopencv_videoio
LIBS += -lv4l2 -lv4l1

HEADERS += \
    inc/backgroundsubtraction.h \
    inc/motionbuffer.h \
    inc/time-stamp.h \
    inc/video-capture-simu.h

SOURCES += \
    src/main-write-to-disk.cpp \
    src/backgroundsubtraction.cpp \
    src/motionbuffer.cpp \
    src/time-stamp.cpp \
    src/video-capture-simu.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
