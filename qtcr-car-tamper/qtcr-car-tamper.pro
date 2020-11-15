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
    ../inc/backgroundsubtraction.h \
    ../inc/motionbuffer.h

SOURCES += \
    ../src/backgroundsubtraction.cpp \
    ../src/buf-main.cpp \
    ../src/main.cpp \
    ../src/motionbuffer.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
