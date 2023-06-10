#pragma once

#include <Windows.h>
#include <vector>
#include <functional>

class Event {
public:
    enum Type {
        GameStart,
        GameEnd,

        TypeNumber,
    };

    using Callback = std::function<void (Type type)>;

    static Event & inst()
    {
        static Event gEvent;
        return gEvent;
    }

    static void add(Type type, Callback cb)
    {
        inst().addListener(type, cb);
    }

    static void trigger(Type type)
    {
        inst().triggerEvent(type);
    }

    Event()
    {
    }

    void addListener(Type type, Callback cb);
    void triggerEvent(Type type);

private:


private:
    std::vector<Callback>       mEvents[int(TypeNumber)];
};
