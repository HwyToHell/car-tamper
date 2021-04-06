#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <string>

bool waitForEnter();

int main_compose_path(int argc, char *argv[])
{
    std::string videoDirAbs = cv::utils::fs::getcwd();
    std::cout << "basePath: " << videoDirAbs << std::endl;

    std::string subDir = "";
    std::string joinedPath = cv::utils::fs::join(videoDirAbs, subDir);
    std::cout << "joinedPath: " << joinedPath << std::endl;

    videoDirAbs = cv::utils::fs::join(videoDirAbs, subDir);
    std::cout << "videoDirAbsJoined: " << videoDirAbs << std::endl;

    std::string baseRelative = "videoDir";
    std::string videoFile = "file.mp4";
    std::string videoDirRel = cv::utils::fs::join(baseRelative, videoFile);
    std::cout << "videoDirRelJoined: " << videoDirRel << std::endl;


    return 0;
    // waitForEnter();
}
