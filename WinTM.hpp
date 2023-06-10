#pragma once

#include <windows.h>
#include <stdint.h>
#include <vector>
#include "D2Utils.hpp"
#include "log.h"
#include "signals.hpp"
#include "TimerQueue.hpp"
#include "Event.hpp"

#define TIME_STEP       (5)
#define TIME_ID         (0x41031202)

class WinTM {
public:
    static WinTM&       inst()
    {
        static WinTM gTM;
        return gTM;
    }

    void        onTimer(uintptr_t nId, bool * blocked)
    {
        if (nId != TIME_ID) {
            return;
        }
        *blocked = true;
        onWinTimer();
    }
    void        addTimer(Timer * t, int timeout);
    void        removeTimer(Timer * t);

private:
    WinTM()
    {
        mIsRunning = false;
        mLastIdle = GetTickCount64();

        Event::add(Event::GameStart, [this](Event::Type) {
            onGameStart();
        });
        Event::add(Event::GameEnd, [this](Event::Type) {
            onGameStop();
        });
    }

    WinTM (const WinTM&) = delete;
    WinTM&  operator=(const WinTM&) = delete;

    static void onGameStartEvent(Event::Type, void * data)
    {
        auto tm = static_cast<WinTM *>(data);
        tm->onGameStart();
    }

    void        onGameStart();
    void        onGameStop();
    bool        start();
    void        stop();
    bool        onWinTimer();


private:
    bool        mIsRunning;
    TimerQueue  mQueue;
    uint64_t    mLastIdle;
};
