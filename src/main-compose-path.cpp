#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <string>

bool waitForEnter();

int main(int argc, char *argv[])
{
    std::string videoDirAbs = cv::utils::fs::getcwd();
    std::string subDir = "sub-Dir";

    std::string joinedPath = cv::utils::fs::join(videoDirAbs, subDir);

    std::cout << "basePath: " << videoDirAbs << std::endl;
    std::cout << "joinedPath: " << joinedPath << std::endl;



    // waitForEnter();
}
