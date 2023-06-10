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

bool WinTM::onWinTimer()
{
    uint64_t now = GetTickCount64();

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
    if (now > mLastIdle + 3000) {
        stop();
        mLastIdle = now;
    }
    return true;
}

void WinTM::addTimer(Timer * timer, int timeout)
{
    timer->expires = GetTickCount64() + timeout;
    mQueue.add(timer);
    if (!mIsRunning) {
        start();
    }
}

void WinTM::removeTimer(Timer *t)
{
    mQueue.remove(t);
}
