#pragma once

#include <functional>
#include <windows.h>
#include "Timer.hpp"

class Task {
public:
    using Complete = std::function<void(Task *)>;

    Task()
    {
        mTimer.cb = onTimeout;
        mTimer.data = this;
        mIsRunning = false;
    }

    virtual ~Task()
    {
    }

    void                run()
    {
        if (mIsRunning) {
            return;
        }
        mIsRunning = true;
        start();
    }

    void                setComplete(Complete cb)
    {
        mComplete = cb;
    }

    void                cancel()
    {
        if (mIsRunning) {
            stop();
        }
    }

    bool                isRunning()
    {
        return mIsRunning;
    }

protected:

    static void         onTimeout(Timer * tm)
    {
        auto task = static_cast<Task *>(tm->data);
        task->execute();
    }

    virtual void        start() = 0;

    virtual void        stop()
    {
        mState = nullptr;
        mTimer.stop();
        mIsRunning = false;
    }

    void                complete()
    {
        stop();
        if (mComplete) {
            mComplete(this);
        }
    }
    void                execute()
    {
        Handler handler = mState;
        mState = nullptr;
        if (handler) {
            (this->*handler)();
        } else {
            volatile int * n = nullptr;
            *n = 1;
        }
    }

    template<typename Func>
    void                setState(const Func& state)
    {
        mState = static_cast<Handler>(state);
    }

    template<typename Func>
    void                next(const Func& state, int timeout=0)
    {
        setState(state);
        mTimer.start(timeout);
    }


protected:
    typedef void        (Task::*Handler)(void);
    Handler             mState;
    Timer               mTimer;
    bool                mIsRunning;
    Complete            mComplete;
};

class ElapsedTime {
public:
    ElapsedTime(): mTime(0)
    {
    }

    bool        isStart() const
    {
        return mTime != 0;
    }

    uint64_t    elapsed() const
    {
        return GetTickCount64() - mTime;
    }

    void        start()
    {
        mTime = GetTickCount64();
    }

private:
    uint64_t    mTime;
};
