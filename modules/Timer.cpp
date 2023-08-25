#include "Timer.hpp"
#include "WinTM.hpp"

void Timer::start(int timeout)
{
    WinTM::inst().addTimer(this, timeout);
}

void Timer::stop()
{
    WinTM::inst().removeTimer(this);
}
