#include "Event.hpp"

void Event::addListener(Type type, Callback cb)
{
    mEvents[type].push_back(cb);
}

void Event::triggerEvent(Type type)
{
    for (auto & cb: mEvents[type]) {
        cb(type);
    }
}
