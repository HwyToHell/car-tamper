// project specific
#include "../inc/motionbuffer.h"
#include "../inc/backgroundsubtraction.h"
#include "../car-tamper-test/inc/video-capture-simu.h"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <iostream>
#include <string>

void printParams(cv::VideoCapture& vidCap) {
    using namespace std;
    //cout << "backend name: " << vidCap.getBackendName() << endl;
    cout << "backend #:    " << vidCap.get(cv::CAP_PROP_BACKEND) << endl;
    cout << "mode:         " << vidCap.get(cv::CAP_PROP_MODE) << endl;
    cout << "fps:          " << vidCap.get(cv::CAP_PROP_FPS) << endl;
    cout << "width:        " << vidCap.get(cv::CAP_PROP_FRAME_WIDTH) << endl;
    cout << "height:       " << vidCap.get(cv::CAP_PROP_FRAME_HEIGHT) << endl << endl;
    return;
}


// test read opencv video capture
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;
    const cv::Scalar red			= cv::Scalar(0,0,255);

    //cout << cv::getBuildInformation() << endl;
    /* test vid cap get and set
    cv::VideoCapture cap;
    cap.open(0);
    printParams(cap);

    bool success = cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
    cout << "width set: " << success << endl;
    success = cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
    cout << "height set: " << success << endl;
    cout << "width:   " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << endl;
    cout << "height:  " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << endl;
    */

    VideoCaptureSimu cap(10);
    MotionBuffer buf(20);
    cv::Mat frame;
    int cnt = 0;

    while (cap.read(frame)) {
        ++cnt;
        cv::imshow("video", frame);

        buf.pushFrameToBuffer(frame);

        if (cnt > 30) {
            buf.toggleSaveToDisk(true);
        }

        if (cnt > 100) {
            buf.toggleSaveToDisk(true);
        }

        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }
    }
    cap.release();
    return 0;

}
