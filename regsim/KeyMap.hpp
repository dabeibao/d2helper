#pragma once

#include <windows.h>
#include <unordered_map>


class KeyMap {
public:
    static KeyMap&      inst()
    {
        static KeyMap   gMap;
        return gMap;
    }

    KeyMap()
    {
        InitializeCriticalSection(&mLock);
    }
    ~KeyMap()
    {
        DeleteCriticalSection(&mLock);
    }

    int                 size() const
    {
        return (int)mPool.size();
    }

    bool                hasKey(HKEY key)
    {
        lock();
        bool ok = mPool.contains((uintptr_t)key);
        unLock();

        return ok;
    }

    bool                remove(HKEY key)
    {
        bool    ok = false;
        lock();
        auto iter = mPool.find((uintptr_t)key);
        if (iter != mPool.end()) {
            mPool.erase(iter);
        }
        unLock();
        return ok;
    }

    void                add(HKEY key)
    {
        lock();
        mPool.insert({(uintptr_t)key, true});
        unLock();
    }

private:
    void                lock()
    {
        EnterCriticalSection(&mLock);
    }
    void                unLock()
    {
        LeaveCriticalSection(&mLock);
    }

private:
    CRITICAL_SECTION                    mLock;
    std::unordered_map<uintptr_t, bool> mPool;
};
