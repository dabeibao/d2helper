#pragma once

#include <Windows.h>
#include <vector>
#include <functional>

class Event {
public:
    enum Type {
        GameStart,
        GameEnd,

        LeftMouseDown,
        RightMouseDown,

        TypeNumber,
    };

    using Callback = std::function<void (Type type, DWORD wParam, DWORD lParam)>;

    static Event & inst()
    {
        static Event gEvent;
        return gEvent;
    }

    static void add(Type type, Callback cb)
    {
        inst().addListener(type, cb);
    }

    static void trigger(Type type, DWORD wParam, DWORD lParam)
    {
        inst().triggerEvent(type, wParam, lParam);
    }

    Event()
    {
    }

    void addListener(Type type, Callback cb);
    void triggerEvent(Type type, DWORD wParam, DWORD lParam);

private:


private:
    std::vector<Callback>       mEvents[int(TypeNumber)];
};
