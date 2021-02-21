#include "../inc/time-stamp.h"

#include <ctime>
#include <iomanip>
#include <sstream>


// Functions //////////////////////////////////////////////////////////////////
std::string getTimeStamp(TimeResolution resolution) {
    // time stamp in std::chrono format
    auto nowTimePoint = std::chrono::system_clock::now();
    auto nowMicroSec = std::chrono::duration_cast<std::chrono::microseconds>
            (nowTimePoint.time_since_epoch());
    auto nowMilliSec =  std::chrono::duration_cast<std::chrono::milliseconds>(nowMicroSec);
    auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(nowMicroSec);

    // convert seconds to time_t for pretty printing with put_time
    std::time_t nowSecTimeT = nowSec.count();

    std::stringstream timeStamp;
    // print with blanks (better reading in console)
    if (resolution == TimeResolution::ms || resolution == TimeResolution::sec) {
        timeStamp << std::put_time(std::localtime(&nowSecTimeT), "%F %T");
    // print w/o blanks (use in file name)
    } else {
        timeStamp << std::put_time(std::localtime(&nowSecTimeT), "%F_%Hh%Mm%Ss");
    }

    if (resolution == TimeResolution::ms) {
        long nowMilliSecRemainder = nowMilliSec.count() % 1000;
        timeStamp << "." << nowMilliSecRemainder;
    }

    // avoid . as ms separator in order to use time stamp as opencv persistence key
    if (resolution == TimeResolution::ms_NoBlank) {
        long nowMilliSecRemainder = nowMilliSec.count() % 1000;
        timeStamp << nowMilliSecRemainder << "ms";
    }

    // add microseconds
    if (resolution == TimeResolution::micSec_NoBlank) {
        long nowMilliSecRemainder = nowMilliSec.count() % 1000;
        long nowMicroSecRemainder = nowMicroSec.count() % 1000;
        timeStamp << nowMilliSecRemainder << "ms" << nowMicroSecRemainder;
    }

    return timeStamp.str();
}


template <typename T> std::chrono::milliseconds asMilliSec(T duration) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}


long elapsedMs(std::chrono::system_clock::time_point startTimePoint) {
    std::chrono::system_clock::time_point endTimePiont = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(endTimePiont - startTimePoint).count();
}

long elapsedMicroSec(std::chrono::system_clock::time_point startTimePoint) {
    std::chrono::system_clock::time_point endTimePiont = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(endTimePiont - startTimePoint).count();
}
