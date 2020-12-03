// project specific


// opencv
#include <opencv2/opencv.hpp>


// std
#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <memory>
#include <string>


// test memory management
int main_mat(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;

    cv::Mat myMat(2,2,CV_8UC3);

    myMat.at<cv::Vec3b>(0,0) = cv::Vec3b(0,1,0);
    myMat.at<cv::Vec3b>(1,1) = cv::Vec3b(0,0,255);
    cout << myMat << endl;
    cout << myMat.at<int>(0) << endl;





    return 0;
}
