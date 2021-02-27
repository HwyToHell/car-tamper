TEMPLATE = app
TARGET = test
QT -= gui

CONFIG += c++14 console
CONFIG -= app_bundle

#define DEBUG_BUILD in debug mode only
#CONFIG(debug, debug|release):DEFINES += DEBUG_BUILD
#message(defines: $$DEFINES)

INCLUDEPATH += $$(HOME)/app-dev/catch/catch_2.8.0
#message(catch dir: $$INCLUDEPATH)

LIBS += -lopencv_core \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_video \
        -lopencv_videoio
LIBS += -lv4l2 -lv4l1

HEADERS += \
    ../inc/backgroundsubtraction.h \
    ../inc/motion-detector.h \
    ../inc/motionbuffer.h \
    ../inc/time-stamp.h \
    ../inc/video-capture-simu.h

SOURCES += \
    ../src/backgroundsubtraction.cpp \
    ../src/motion-detector.cpp \
    ../src/motionbuffer.cpp \
    ../src/time-stamp.cpp \
    ../src/video-capture-simu.cpp \
    main-test-cases.cpp \
    motion-detector-test.cpp \
    motionbuffer-test.cpp \
    vid-cap-simu-test.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
