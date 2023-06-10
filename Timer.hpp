#pragma once

#include <stdint.h>
#include <functional>

struct Timer {
    using Callback = std::function<void (Timer *)>;
    Timer(uint64_t aExpires = 0): expires(aExpires), _index(0)
    {
    }

    void        start(int timeout);
    void        stop();

    bool        isPending() const
    {
        return _index != 0;
    }

    size_t      index() const {
        return _index;
    }

    Callback    cb;
    void *      data;
    uint64_t    expires;

    friend class TimerQueue;

private:
    size_t      _index;
};
