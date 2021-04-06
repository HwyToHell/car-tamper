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
int main_video_writer(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;


    cout << cv::getBuildInformation() << endl;
    cv::VideoCapture cap;
    cap.open(0);
    printParams(cap);

    bool success = cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cout << "width set: " << success << endl;
    success = cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cout << "height set: " << success << endl;
    cout << "width:   " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << endl;
    cout << "height:  " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << endl;


    cv::Mat frame;
    int cnt = 0;

    cv::VideoWriter writer;

    int codec_h264 = cv::VideoWriter::fourcc('X','2','6','4');
    int codec_mjpg =  cv::VideoWriter::fourcc('M','J','P','G');
    int codec_xvid = cv::VideoWriter::fourcc('X','V','I','D');
    cv::Size frameSize(cap.get(cv::CAP_PROP_FRAME_WIDTH), cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    if (writer.open("out_xvid.avi", codec_xvid, 30, frameSize)) {
        cout << "writer open" << endl;
        cout << writer.getBackendName() << endl;
        cout << "quality: " << writer.get(cv::VIDEOWRITER_PROP_QUALITY) << endl;
        cout << "bytes:   " << writer.get(cv::VIDEOWRITER_PROP_FRAMEBYTES) << endl << endl;
    } else {
        cout << "cannot open writer" << endl;
    }

    //return 0;

    while (cap.read(frame)) {
        ++cnt;
        cv::imshow("video", frame);
        writer.write(frame);
        // std::string imgFile = getTimeStampMs() + ".jpg";
        // cv::imwrite(imgFile, frame);

        cout << "frame:   " << cnt << endl;
        cout << "quality: " << writer.get(cv::VIDEOWRITER_PROP_QUALITY) << endl;
        cout << "bytes:   " << writer.get(cv::VIDEOWRITER_PROP_FRAMEBYTES) << endl << endl;



        if (cnt >= 100) {
            break;
        }



        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }


    }
    cout << "end reading frames " << endl;

    //buf.printBuffer();

    // videoWriter.release();
    //cap.release();
    return 0;

}
