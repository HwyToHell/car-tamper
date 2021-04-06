#include "../inc/video-capture-simu.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

bool waitForEnter();



// test cv::VideoCapture (read from web cam), get and set parameters,
// write video with cv::VideoWriter
int main_video_writer(int argc, char *argv[])
{
    (void)argc;
    using namespace std;
    string videoFile = argv[1];
    cout << "video file to process: " << videoFile << endl;


    //cout << cv::getBuildInformation() << endl;
    cv::VideoCapture cap;
    cap.open(0);

    if (cap.isOpened()) {
        printParams(cap);
    }


   // return 0;



    cv::Mat frame;
    int cnt = 0;

    cv::VideoWriter writer;


    int codec_x263 = cv::VideoWriter::fourcc('X','2','6','3');
    int codec_mjpg = cv::VideoWriter::fourcc('M','J','P','G');
    int codec_h264 = cv::VideoWriter::fourcc('X','2','6','4');
    int codec_xvid = cv::VideoWriter::fourcc('X','V','I','D');

    cv::Size frameSize(cap.get(cv::CAP_PROP_FRAME_WIDTH), cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    if (writer.open("out_xvid.avi", codec_xvid, 30, frameSize))  {
        std::cout << "writer open" << std::endl;
        std::cout << writer.getBackendName() << std::endl;
    } else {
        std::cout << "cannot open writer" << std::endl;
    }


    while (cap.read(frame)) {
        ++cnt;
        //cout << endl << "frame: " << cnt << endl;

        cv::imshow("video", frame);
        writer.write(frame);
        std::cout << "frame:   " << cnt << std::endl;
        std::cout << "quality: " << writer.get(cv::VIDEOWRITER_PROP_QUALITY) << std::endl;
        std::cout << "bytes:   " << writer.get(cv::VIDEOWRITER_PROP_FRAMEBYTES) << std::endl << std::endl;

        if (cnt >= 100) {
            break;
        }

        if (cv::waitKey(10) == 27) {
            std::cout << "esc -> end video processing" << std::endl;
            break;
        }


    }
    std::cout << "end reading frames " << endl;

    //buf.printBuffer();

    writer.release();

    cap.release();
    return 0;

}
