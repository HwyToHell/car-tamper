TEMPLATE = app
TARGET = tamper
QT -= gui

CONFIG += c++11 console
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
    ../inc/motionbuffer.h

SOURCES += \
    ../car-tamper-test/src/video-capture-simu.cpp \
    ../src/backgroundsubtraction.cpp \
    ../src/main-write.cpp \
    ../src/motionbuffer.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
