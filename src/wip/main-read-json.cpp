// project specific
#include "../inc/backgroundsubtraction.h"
#include "../inc/motionbuffer.h"
#include "../inc/time-stamp.h"
#include "../inc/video-capture-simu.h"

// opencv
#include <opencv2/opencv.hpp>

// qt
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QString>

// std
#include <iostream>
#include <string>
#include <vector>

std::vector<int> getBufferSamples(cv::FileStorage fs, std::string key) {
    std::vector<int> bufSamples;
    cv::FileNode root = fs.root();
    if (root.size() < 1) {
        std::cout << "root file node empty" << std::endl;
    }

    // key available?
    cv::FileNodeIterator itFirstNode = root.begin();
    std::vector<cv::String> keys = (*itFirstNode).keys();
    bool isKeyFound = false;
    for (auto k : keys) {
        if (k.compare(key) == 0) {
            isKeyFound = true;
        }
    }
    if (!isKeyFound) {
        std::cout << "file node does not contain key: " << key << std::endl;
        return bufSamples;
    }

    // populate vector with values associated with key
    for (cv::FileNodeIterator itSample = root.begin(); itSample != root.end(); ++itSample) {
        bufSamples.push_back(static_cast<int>((*itSample)[key]));
    }

    return bufSamples;
}





// test MotionBuffer saveToDisk
int main_read_json(int argc, char *argv[]) {
    //QApplication a(argc, argv);
    using namespace std;

    /*
    QString paramFile = QFileDialog::getOpenFileName(nullptr,
         "Select json file",
         QDir::currentPath(),
        "Video files (*.json)" );
    std::string file(paramFile.toUtf8());
    cout << file << endl;
    */
    //std::string file = "log/2021-02-07_19h17m41s514ms.json";
    std::string file = "log/2021-02-08_21h29m34s57ms.json";

    /*
    cv::FileStorage fs;
    fs.open(file, cv::FileStorage::Mode::READ);
    if (!fs.isOpened()) {
        cout << "file storage not open"<< endl;
        return -1;
    }
    */
    cv::FileStorage fs(file, cv::FileStorage::Mode::READ);

    // starting from root file node step down to other nodes
    cv::FileNode node = fs.root();
    cout << "root:    " << node.name() << endl;
    cout << "size:    " << node.size() << endl;
    cout << "empty:   " << node.empty() << endl;
    cout << "isMap:   " << node.isMap() << endl;
    cout << "isNamed: " << node.isNamed() << endl;
    cout << "isNone:  " << node.isNone() << endl << endl;


    for (cv::FileNodeIterator it = node.begin(); it != node.end(); ++it) {
        cv::FileNode subNode = *it;
        cout << "sub:     " << (*it).name() << endl;
        cout << "size:    " << (*it).size() << endl;
        cout << "empty:   " << (*it).empty() << endl;
        cout << "isMap:   " << (*it).isMap() << endl;
        cout << "isNamed: " << (*it).isNamed() << endl;
        cout << "isNone:  " << (*it).isNone() << endl << endl;

        for (cv::FileNodeIterator itSub = subNode.begin(); itSub != subNode.end(); ++itSub) {
            cout << "sub sub: " << (*itSub).name() << endl;
            cout << "size:    " << (*itSub).size() << endl;
            cout << "isMap:   " << (*itSub).isMap() << endl;
            cout << "isNamed: " << (*itSub).isNamed() << endl;
            cout << "isInt:   " << (*itSub).isInt() << endl;
            int val = 0;
            cv::read(*itSub, val, 0);
            cout << "value:   " << val << endl << endl;
        }
    }

    // extract frame count and time stamp
    vector<int> frameCounts = getBufferSamples(fs, "frame count");
    cout << "frame counts" << endl;
    for (auto fcnt : frameCounts)
         cout << fcnt << endl;

    vector<int> timeStamps = getBufferSamples(fs, "time stamp");
    cout << "time stamps" << endl;
    for (auto tstmp : timeStamps)
         cout << tstmp << endl;

    return 0;
}
