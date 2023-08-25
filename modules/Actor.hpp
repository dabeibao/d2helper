#pragma once

#include "Timer.hpp"

class Actor {
public:
    Actor()
    {
        mTimer.cb = onTimeout;
        mTimer.data = this;
    }

    virtual ~Actor()
    {
    }

    virtual void        start() { }

protected:
    static void         onTimeout(Timer * tm)
    {
        auto actor = static_cast<Actor *>(tm->data);
        actor->execute();
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
    typedef void        (Actor::*Handler)(void);
    int                 mStatus;
    Handler             mState;
    Timer               mTimer;
};
