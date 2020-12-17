// std
#include <iostream>
#include <string>
// qt
#include <QFile>
#include <QString>
// opencv
#include <opencv2/opencv.hpp>
// project specific
#include "../../cpp/inc/opencv/backgroundsubtraction.h"


// test read opencv video capture
int main_read(int argc, char *argv[])
{
    using namespace std;
    const cv::Scalar red			= cv::Scalar(0,0,255);

    if (argc <= 1 ) {
        cout << "usage: " << argv[0] << " video-file-name" << endl;
        return -1;
    }

    QFile videoFile(QString::fromUtf8(argv[1]));
    if (!videoFile.exists()) {
        cout << "video input file does not exist" << endl;
        return -2;
    }

    cv::VideoCapture cap(videoFile.fileName().toStdString());
    cv::Mat frame, postFrame, motionFrame;
    BackgroundSubtractorLowPass lowPass(0.005, 40);


    while (cap.read(frame)) {

        cv::blur(frame, postFrame, cv::Size(10,10));
        lowPass.apply(postFrame, motionFrame);

        int motionIntensity = cv::countNonZero(motionFrame);
        cout << motionIntensity << endl;

        if (motionIntensity > 100) {
            cv::putText(frame, "Achtung Bewegung!", cv::Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 2, red, 3);
            // if (cv::waitKey(100) == 27)
            //    break;
        }

        cv::imshow("video", frame);
        //cv::imshow("blurred", postFrame);
        cv::imshow("motion", motionFrame);


        if (cv::waitKey(10) == 27) {
            cout << "esc -> end video processing" << endl;
            break;
        }
    }
    cap.release();
    return 0;

}
