#include <windows.h>
#include <commctrl.h>
#include <list>
#include "Define.h"
#include "d2h_module.hpp"
#include "d2ptrs.h"
#include "KeyModule.hpp"
#include "d2vars.h"
#include "log.h"
#include "D2Utils.hpp"
#include "Define.h"
#include "Actor.hpp"
#include "D2CallStub.hpp"
#include "Event.hpp"

struct SkillTime {
    int         key;
    int         skillId;
    uint64_t    startTime;

    bool        isExpired() const
    {
        return startTime + 1000 < GetTickCount64();
    }

    bool        isValid() const
    {
        return skillId != -1;
    }
};

//#define DEBUG_FAST_CAST

#ifdef DEBUG_FAST_CAST
#define fcDbg(fmt, ...)         D2Util::showInfo(fmt, ##__VA_ARGS__)
#define trace(fmt, ...)         do { log_trace(fmt, ##__VA_ARGS__); log_flush(); } while (0)
#else
#define fcDbg(fmt, ...)
#define trace(fmt, ...)         log_verbose(fmt, ##__VA_ARGS__)
#endif


static bool isFastCastAble()
{
    if (D2Util::getState() != D2Util::ClientStateInGame) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_CURRSKILL)) {
        fcDbg(L"Curr Skill");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_CHATINPUT)) {
        fcDbg(L"Input");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_INTERACT)) {
        fcDbg(L"INTERACT");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_GAMEMENU)) {
        fcDbg(L"Game Menu");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_NPCTRADE)) {
        fcDbg(L"Trade");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_MODITEM)) {
        fcDbg(L"Mod");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_PPLTRADE)) {
        fcDbg(L"PP Trade");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_MSGLOG)) {
        fcDbg(L"MSG");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_STASH)) {
        fcDbg(L"Stash");
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_CUBE)) {
        fcDbg(L"CUBE");
        return false;
    }
    bool left = D2Util::uiIsSet(UIVAR_STATS) || D2Util::uiIsSet(UIVAR_QUEST) || D2Util::uiIsSet(UIVAR_PET);
    bool right = D2Util::uiIsSet(UIVAR_INVENTORY) || D2Util::uiIsSet(UIVAR_SKILLS);
    if (left && right) {
        fcDbg(L"FULL");
        return false;
    }
    return true;
}

class Watcher {
public:
    virtual void signal(Actor * actor) = 0;
};

class RunActor: public Actor {
public:
    RunActor(Watcher * watcher): Actor(), mWatcher(watcher), mIsRunning(false)
    {
    }

    void startSkill(const SkillTime& skill)
    {
        mIsRunning = true;
        mSkill = skill;
        run();
    }

    void cancel()
    {
        if (mIsRunning) {
            mTimer.stop();
            mIsRunning = false;
        }
    }

private:
    void run()
    {
        int origId = D2Util::getRightSkillId();

        trace("%s: Start skill %d, orig %d\n", __FUNCTION__, mSkill.skillId, origId);
        if (origId == mSkill.skillId) {
            fcDbg(L"no change required orig %d, cur %d", origId, mSkill.skillId);
            return sendMouseDown();
        }
        fcDbg(L"Set Skill to %d cur %d", mSkill.skillId, origId);
        DefSubclassProc(origD2Hwnd, WM_KEYDOWN, mSkill.key, 0);
        DefSubclassProc(origD2Hwnd, WM_KEYUP, mSkill.key, 0);
        D2Util::setSkill(mSkill.skillId);
        mRetryCount = 0;
        next(&RunActor::checkSkill, 5);
    }

    void checkSkill()
    {
        if (!isFastCastAble()) {
            return done();
        }

        int skillId = D2Util::getRightSkillId();

        trace("%s: wait skill %d, cur %d\n", __FUNCTION__, mSkill.skillId, skillId);
        if (skillId != mSkill.skillId) {
            mRetryCount += 1;
            if (mRetryCount >= 50) { // 250ms
                log_verbose("Failed to set skill id to %d, cur %d\n", mSkill.skillId, skillId);
                return done();
            }
            return next(&RunActor::checkSkill, 5);
        }
        fcDbg(L"Skill %d set", mSkill.skillId);
        trace("%s: wait skill %d done\n", __FUNCTION__, mSkill.skillId);
        //next(&RunActor::sendMouseDown, 0);
        sendMouseDown();
    }

    void sendMouseDown()
    {
        POINT screenPos = { MOUSE_POS->x, MOUSE_POS->y};
        mCurrentPos = screenPos;
        //D2Util::screenToAutoMap(&screenPos, &mCurrentPos);
        trace("%s: send mouse event skill %d cur %d\n", __FUNCTION__,
              mSkill.skillId, D2Util::getRightSkillId());
        SendMessage(origD2Hwnd, WM_RBUTTONDOWN, MK_RBUTTON,
                    ((DWORD)mCurrentPos.x) | (((DWORD)mCurrentPos.y) << 16));
        SendMessage(origD2Hwnd, WM_RBUTTONUP, MK_RBUTTON,
                    ((DWORD)mCurrentPos.x) | (((DWORD)mCurrentPos.y) << 16));
        next(&RunActor::done, 10);
    }

    void sendMouseUp()
    {
        SendMessage(origD2Hwnd, WM_RBUTTONUP, MK_RBUTTON,
                    ((DWORD)mCurrentPos.x) | (((DWORD)mCurrentPos.y) << 16));
        next(&RunActor::done, 5);
        trace("%s: skill %d mouse sent\n", __FUNCTION__, mSkill.skillId);
    }

    void done()
    {
        mIsRunning = false;
        mWatcher->signal(this);
    }

private:
    Watcher *   mWatcher;
    bool        mIsRunning;
    SkillTime   mSkill;
    int         mRetryCount;
    POINT       mCurrentPos;
};

class FastCastActor: public Watcher {
    enum State {
        Idle,
        Skill,
        Restore,
        WaitRestore,
    };

    enum Result {
        Continue,
        Wait,
    };

public:
    static FastCastActor& inst()
    {
        static FastCastActor gFastCastActor;
        return gFastCastActor;
    }

    FastCastActor(): mState(Idle), mCrankCount(0), mIsRunning(false), mActor(this),
                     mOrigSkillId(-1), mWaitRestoreStart(0)
    {
        auto f = [this](Event::Type) { stop(); };
        Event::add(Event::GameStart, f);
        Event::add(Event::GameEnd, f);

        mRestoreDelayTimer.cb = [this](Timer *) {
            crank();
        };
    }

    void startSkill(const SkillTime& newSkill)
    {
        for (auto& sk: mPendingSkills) {
            if (sk.skillId == newSkill.skillId) {
                sk.startTime = newSkill.startTime;
                return;
            }
        }
        mPendingSkills.push_back(newSkill);

        crank();
    }


private:
    virtual void signal(Actor * actor) override
    {
        mIsRunning = false;
        crank();
    }

    void crank()
    {
        int cur = mCrankCount;

        mCrankCount += 1;
        if (cur != 0) {
            return;
        }

        for (; mCrankCount > 0; mCrankCount -= 1) {
            trace("Start crank cur %d, count %d\n", cur, mCrankCount);
            doCrank();
        }
    }

    void doCrank()
    {
        Result res = Continue;

        while (res != Wait) {
            switch (mState) {
            case Idle:
                res = handleIdle();
                break;
            case Skill:
                res = handleSkill();
                break;
            case Restore:
                res = handleRestore();
                break;
            case WaitRestore:
                res = handleWaitRestore();
                break;
            default:
                log_trace("Unexpected state %d\n", mState);
                mState = Idle;
                return;
            }
        }
    }

    Result handleIdle()
    {
        trace("IDLE: empty %d\n", mPendingSkills.empty());
        if (mPendingSkills.empty()) {
            return Wait;
        }
        mState = Skill;
        return Continue;
    }

    Result handleSkill()
    {
        trace("SKILL: running %d empty %d\n", mIsRunning, mPendingSkills.empty());
        if (mIsRunning) {
            return Wait;
        }
        if (mPendingSkills.empty()) {
            mState = Restore;
            return Continue;
        }

        auto st = mPendingSkills.front();
        mPendingSkills.pop_front();
        runSkill(st);
        return Wait;
    }

    Result handleRestore()
    {
        trace("Restore: orig %d\n", mOrigSkillId);
        if (!mPendingSkills.empty()) {
            mState = Skill;
            return Continue;
        }
        if (mOrigSkillId == -1) {
            mState = Idle;
            return Continue;
        }
        mState = WaitRestore;
        return Continue;
    }

    Result handleWaitRestore()
    {
        trace("WaitRestore: orig %d\n", mOrigSkillId);
        if (!mPendingSkills.empty()) {
            finishWaitRestore();
            mState = Skill;
            return Continue;
        }

        if (mOrigSkillId == -1) {
            finishWaitRestore();
            mState = Idle;
            return Continue;
        }

        if (mWaitRestoreStart == 0) {
            mWaitRestoreStart = GetTickCount64();
            mRestoreDelayTimer.start(200);
            return Wait;
        }

        if (mRestoreDelayTimer.isPending()) {
            return Wait;
        }

        if (D2Util::getRightSkillId() != mOrigSkillId) {
            // Retry
            trace("WaitRestore: set kill to %d cur %d\n", mOrigSkillId, D2Util::getRightSkillId());
            D2Util::setSkill(mOrigSkillId);
            mWaitRestoreStart = 0;
            return Continue;
        }

        // User is selecting a new skill
        if (D2Util::uiIsSet(UIVAR_CURRSKILL)) {
            mOrigSkillId = -1;
            mState = Idle;
            finishWaitRestore();
            return Continue;
        }

        // Fix SK triggered by hackmap
        // This possible happens in 3 seconds.
        // Still some very corner case that fails to restore, but good enough now.
        uint64_t now = GetTickCount64();
        if (now < mWaitRestoreStart + 30000) {
            mRestoreDelayTimer.start(10);
            return Wait;
        }


        mOrigSkillId = -1;
        mState = Idle;
        finishWaitRestore();
        return Continue;
    }

    void finishWaitRestore()
    {
        mWaitRestoreStart = 0;
        mRestoreDelayTimer.stop();
        mIsRunning = false;
    }

    void runSkill(const SkillTime& st)
    {
        if (mOrigSkillId == -1) {
            mOrigSkillId = D2Util::getRightSkillId();
        }
        mIsRunning = true;
        mActor.startSkill(st);
    }


    void  stop()
    {
        mActor.cancel();

        mState = Idle;
        mCrankCount = 0;
        mPendingSkills.clear();
        mIsRunning = false;
        mOrigSkillId = -1;
        mWaitRestoreStart = 0;
        mRestoreDelayTimer.stop();
    }

    bool isRunning() const
    {
        return mIsRunning != 0;
    }

private:
    State               mState;
    int                 mCrankCount;

    std::list<SkillTime> mPendingSkills;
    bool                mIsRunning;
    RunActor            mActor;

    int                 mOrigSkillId;
    uint64_t            mWaitRestoreStart;
    Timer               mRestoreDelayTimer;
};

static bool doFastCast(BYTE key, BYTE repeat)
{
    if (!isFastCastAble()) {
        D2Util::showVerbose(L"Not in Game: %d, repeat %d", D2CheckUiStatus(0), repeat);
        return false;
    }
    fcDbg(L"key: %d, repeat %d", key, repeat);
    int         func = D2Util::getKeyFunc(key);

    if (func < 0) {
        return false;
    }

    int         skillId = D2Util::getSkillId(func);
    if (skillId < 0) {
        fcDbg(L"KEY %u -> %u (unbind)", key, func);
        return false;
    }

    fcDbg(L"KEY %u -> %u -> skill %u\n", key, func, skillId);

    FastCastActor::inst().startSkill({key, skillId, GetTickCount64()});

    return true;
}

static int fastCastModuleInstall()
{
    keyRegisterHandler(doFastCast);
    FastCastActor::inst();
    return 0;
}

static void fastCastModuleUninstall()
{
}

static void fastCastModuleOnLoad()
{
}

static void fastCastModuleReload()
{
}


D2HModule fastCastModule = {
    "Fast Cast",
    fastCastModuleInstall,
    fastCastModuleUninstall,
    fastCastModuleOnLoad,
    fastCastModuleReload,
};
