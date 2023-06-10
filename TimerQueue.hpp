#pragma once

#include <vector>

#include "Timer.hpp"
class TimerQueue {
    static constexpr size_t Heap0 = 1;

    static size_t heapParent(size_t k)
    {
        return k / 2;
    }

    static bool heapDone(int p, int k)
    {
        (void)k;
        return p == 0;
    }

public:
    explicit TimerQueue(): mTimers(1)
    {
    }

    Timer * pop()
    {
        Timer * t = first();
        remove(t);
        return t;
    }

    void clear()
    {
        mTimers.resize(Heap0);
    }

    Timer * first() const
    {
        return mTimers[Heap0];
    }

    bool isEmpty() const
    {
        return mTimers.size() == Heap0;
    }

    size_t count() const
    {
        return mTimers.size() - Heap0;
    }

    void add(Timer * t)
    {
        if (t->_index != 0) {
            remove(t->_index);
            t->_index = 0;
        }
        mTimers.push_back(t);
        up(mTimers.size() - 1);
    }

    void remove(Timer * t)
    {
        if (t->_index == 0) {
            return;
        }
        remove(t->_index);
        t->_index = 0;
    }

private:
    explicit TimerQueue(const TimerQueue&) = delete;
    TimerQueue operator=(const TimerQueue&) = delete;

    void down(size_t k)
    {
        Timer * e = mTimers[k];

        for (;;) {
            size_t child = k * 2;
            if (child >= mTimers.size()) {
                break;
            }
            if (child + 1 < mTimers.size()) {
                if (mTimers[child]->expires > mTimers[child +1]->expires) {
                    child = child + 1;
                }
            }
            if (e->expires <= mTimers[child]->expires) {
                break;
            }
            mTimers[k] = mTimers[child];
            mTimers[k]->_index = k;
            k = child;
        }
        mTimers[k] = e;
        e->_index = k;
    }

    void up(size_t k)
    {
        Timer * e = mTimers[k];
        for (;;) {
            size_t p = heapParent(k);
            if (p == 0) {
                break;
            }
            if (mTimers[p]->expires <= e->expires) {
                break;
            }
            // down parent
            mTimers[k] = mTimers[p];
            mTimers[k]->_index = k;
            k = p;
        }
        mTimers[k] = e;
        mTimers[k]->_index = k;
    }

    void remove(size_t k)
    {
        size_t size = mTimers.size();
        size -= 1;
        Timer * last = mTimers[size];
        mTimers.pop_back();
        if (k < size) {
            mTimers[k] = last;
            adjust(k);
        }
    }

    void adjust(size_t k)
    {
        if (k > Heap0) {
            size_t p = heapParent(k);
            if (mTimers[k]->expires < mTimers[p]->expires) {
                up(k);
                return;
            }
        }
        down(k);
    }


private:
    std::vector<Timer *>        mTimers;
};
