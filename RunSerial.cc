#include <Magick++.h>
#include <tuple>

#include "Timer.hpp"

std::tuple<double, double>
runSerial()
{
    Timer clockIM;
    clockIM.start();
    runIMSerial();
    clockIM.stop();

    Timer clockSerial;
    clockSerial.start();
    ourSerial();
    clockSerial.stop();

}



double
startIMSerial()
{
    Timer clock;
    clock.start();
    runIMSerial();
    clock.stop();
}


runIMSerial()
{
    oilPaint(3.0);
}