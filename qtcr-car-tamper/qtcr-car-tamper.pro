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
    ../car-tamper-test/inc/video-capture-simu.h \
    ../inc/backgroundsubtraction.h \
    ../inc/motionbuffer.h \
    ../inc/time-stamp.h

SOURCES += \
    ../car-tamper-test/src/video-capture-simu.cpp \
    ../src/backgroundsubtraction.cpp \
    ../src/main-mat.cpp \
    ../src/main-read.cpp \
    ../src/main-verify-write.cpp \
    ../src/main-write.cpp \
    ../src/motionbuffer.cpp \
    ../src/time-stamp.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
