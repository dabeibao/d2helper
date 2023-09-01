#include "Event.hpp"

void Event::addListener(Type type, Callback cb)
{
    mEvents[type].push_back(cb);
}

void Event::triggerEvent(Type type)
{
    // Trigger stop event in reversed order
    if (type == GameEnd) {
        for (int i = mEvents[type].size() - 1; i >= 0; i-= 1) {
            mEvents[type][i](type);
        }
        return;
    }

    for (auto& cb: mEvents[type]) {
        cb(type);
    }
}
