#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>


std::string getTimeStamp() {
    // time stamp in std::chrono format
    auto nowTimePoint = std::chrono::system_clock::now();
    auto nowMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>
            (nowTimePoint.time_since_epoch());
    auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(nowMilliSec);

    // convert seconds to time_t for pretty printing with put_time
    std::time_t nowSecTimeT = nowSec.count();
    long nowMilliSecRemainder = nowMilliSec.count() % 1000;

    std::stringstream timeStamp;
    timeStamp << std::put_time(std::localtime(&nowSecTimeT), "%F %T")
              << "." << nowMilliSecRemainder;
    return timeStamp.str();
}

int clock-main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    using namespace std;

    auto nowTimePoint = chrono::system_clock::now();
    time_t now_time_t = chrono::system_clock::to_time_t(nowTimePoint);
    cout << "to_time_t now: " << chrono::system_clock::to_time_t(nowTimePoint) << endl;
    cout << "localtime now: " << localtime(&now_time_t) << endl;
    cout << "put_time  now: " << put_time(localtime(&now_time_t), "%F %T") << endl;
    cout << "time since epoch in sec: "
         << chrono::duration_cast<chrono::seconds>(nowTimePoint.time_since_epoch()).count()
         << endl;
    cout << "time since epoch in ms:  "
         << chrono::duration_cast<chrono::milliseconds>(nowTimePoint.time_since_epoch()).count()
         << endl;

    cout << "getTimeStamp: " << getTimeStamp() << endl;
}
