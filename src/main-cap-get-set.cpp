// project specific
#include "../inc/motionbuffer.h"
#include "../inc/backgroundsubtraction.h"
#include "../inc/video-capture-simu.h"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <iostream>
#include <string>

void printParams(cv::VideoCapture& vidCap) {
    using namespace std;
    cout << "backend name: " << vidCap.getBackendName() << endl;
    cout << "backend #:    " << vidCap.get(cv::CAP_PROP_BACKEND) << endl;
    cout << "mode:         " << vidCap.get(cv::CAP_PROP_MODE) << endl;
    cout << "fps:          " << vidCap.get(cv::CAP_PROP_FPS) << endl;
    cout << "width:        " << vidCap.get(cv::CAP_PROP_FRAME_WIDTH) << endl;
    cout << "height:       " << vidCap.get(cv::CAP_PROP_FRAME_HEIGHT) << endl << endl;
    return;
}


// test cv::VideoCapture (read from web cam), get and set parameters,
// write video with cv::VideoWriter
int main_cap_get_set(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;

    /*
     * test vid cap get and set

    cout << cv::getBuildInformation() << endl;
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

    size_t fps = 60;
    VideoCaptureSimu cap(InputMode::videoFile, "160x120", fps, false);
    cout << "get:  " << cap.get(cv::CAP_PROP_MODE) << endl;
    cout << "cast: " << static_cast<double>(InputMode::videoFile) << endl;


    cv::Mat frame;
    int cnt = 0;

    MotionBuffer buf(30, fps, "videos", "log", true);

    while (cap.read(frame)) {
        ++cnt;
        cv::imshow("video", frame);
        // videoWriter.write(frame);
        // std::string imgFile = getTimeStampMs() + ".jpg";
        // cv::imwrite(imgFile, frame);

        // test buffer functions
        cout << endl << "frame: " << cnt << endl;

        buf.pushToBuffer(frame);


        if (cnt >= 40 && cnt <= 41) {
            //cap.setMode(GenMode::motionAreaAndTime, 50, 30);
            buf.setSaveToDisk(true);
        }

        if (cnt >= 70 && cnt <= 71) {
            //cap.setMode(GenMode::motionAreaAndTime, 50, 80);
            buf.setSaveToDisk(false);
        }

        if (cnt >= 110) {
            break;
        }



        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }


    }
    std::string videoFileName = buf.getVideoFileName();
    cout << "videoFileName: " << videoFileName << endl;
    cout << "end reading frames " << endl;

    //buf.printBuffer();

    // videoWriter.release();
    //cap.release();
    return 0;

}
