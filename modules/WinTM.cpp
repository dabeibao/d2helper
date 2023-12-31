#include "WinTM.hpp"
#include "D2Utils.hpp"
#include "KeyModule.hpp"

bool WinTM::start()
{
    if (mIsRunning) {
        //D2Util::showVerbose(L"is Running");
        return true;
    }

    D2Util::showVerbose(L"start timer");
    mIsRunning = true;
    SetTimer(origD2Hwnd, TIME_ID, TIME_STEP, nullptr);
    return true;
}

void WinTM::stop()
{
    if (mIsRunning) {
        KillTimer(origD2Hwnd, TIME_ID);
        mIsRunning = false;
    }
}

void WinTM::onGameStart()
{
    D2Util::showVerbose(L"Game start!!!");
    stop();
}

void WinTM::onGameStop()
{
    stop();
}

bool WinTM::checkTimer()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    double now = (double)count.QuadPart;

    while (!mQueue.isEmpty()) {
        auto t = mQueue.first();
        if (t->expires > now) {
            break;
        }
        mQueue.pop();
        t->cb(t);
        mLastIdle = now;
    }
    if (!mQueue.isEmpty()) {
        return true;
    }
    if (now > mLastIdle + 3 * mFreq) {
        stop();
        mLastIdle = now;
    }
    return true;
}

void WinTM::addTimer(Timer * timer, int timeout)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double count = mFreq * timeout / 1000.0;
    timer->expires = now.QuadPart + count;
    mQueue.add(timer);
    if (!mIsRunning) {
        start();
    }
}

void WinTM::removeTimer(Timer *t)
{
    mQueue.remove(t);
}
