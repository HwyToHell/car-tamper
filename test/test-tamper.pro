TEMPLATE = app
TARGET = test
QT -= gui

CONFIG += c++14 console
CONFIG -= app_bundle

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
    ../inc/motionbuffer.h \
    ../inc/time-stamp.h \
    ../inc/video-capture-simu.h

SOURCES += \
    ../src/motionbuffer.cpp \
    ../src/time-stamp.cpp \
    ../src/video-capture-simu.cpp \
    main-test-cases.cpp \
    motionbuffer-test.cpp \
    vid-cap-simu-test.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
