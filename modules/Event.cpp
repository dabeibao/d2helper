#include "Event.hpp"

void Event::addListener(Type type, Callback cb)
{
    mEvents[type].push_back(cb);
}

void Event::triggerEvent(Type type, DWORD wParam, DWORD lParam)
{
    // Trigger stop event in reversed order
    if (type == GameEnd) {
        for (int i = mEvents[type].size() - 1; i >= 0; i-= 1) {
            mEvents[type][i](type, wParam, lParam);
        }
        return;
    }

    for (auto& cb: mEvents[type]) {
        cb(type, wParam, lParam);
    }
}
