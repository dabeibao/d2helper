#include <windows.h>
#include <commctrl.h>
#include <list>
#include <set>
#include "Define.h"
#include "d2h_module.hpp"
#include "d2ptrs.h"
#include "KeyModule.hpp"
#include "d2vars.h"
#include "log.h"
#include "D2Utils.hpp"
#include "Define.h"
#include "Task.hpp"
#include "D2CallStub.hpp"
#include "Event.hpp"
#include "cConfigLoader.hpp"
#include "hotkey.hpp"

struct SkillTime {
    int         key;
    int         skillId;
    bool        isLeft;
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

#define fcDbg(fmt, ...)         do { if (fastCastDebug) D2Util::showInfo(fmt, ##__VA_ARGS__); } while(0)
#define trace(fmt, ...)         do { if (fastCastDebug) log_trace(fmt, ##__VA_ARGS__); log_flush(); } while (0)

static bool fastCastDebug;
static bool fastCastEnabled;
static uint32_t fastCastToggleKey = Hotkey::Invalid;
static std::set<int> fastCastLeftHoldSkills;

static WNDPROC getOrigProc(HWND hwnd)
{
    TCHAR       className[64];
    int         classNameLen;
    WNDCLASS    classInfo;
    HINSTANCE   hInst = GetModuleHandle(NULL);
    WNDPROC     orig;

    if (hInst == NULL) {
        return NULL;
    }
    // Get "Diablo II" class name
    classNameLen = GetClassName(hwnd, &className[0], sizeof(className) / sizeof(className[0]));
    if (classNameLen == 0) {
        trace("Failed to get class name\n");
        return FALSE;
    }
    if (!GetClassInfo(hInst, className, &classInfo)) {
        trace("Failed to get class info\n");
        return FALSE;
    }
    orig = classInfo.lpfnWndProc;
    trace("Orig proc: %p\n", orig);
    return orig;
}

__declspec(noinline)
LRESULT WINAPI mySendMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static WNDPROC      origProc;
    static BOOL         isGot;

    LRESULT             ret;

    trace("Get message 0x%x param 0x%x 0x%x\n", msg, wParam, lParam);
    if (!isGot) {
        origProc = getOrigProc(hWnd);
        isGot = TRUE;
    }
    if (origProc != NULL) {
        ret = CallWindowProcA(origProc, hWnd, msg, wParam, lParam);
    } else {
        ret = SendMessageA(hWnd, msg, wParam, lParam);
    }

    return ret;
}

static bool isFastCastAble()
{
    if (!fastCastEnabled) {
        return false;
    }
    return D2Util::isGameScreen();
}

class RunTask: public Task {
public:
    RunTask()
    {
    }

    void startSkill(const SkillTime& skill)
    {
        if (mIsRunning) {
            cancel();
        }
        mSkill = skill;
        run();
    }

private:
    bool                isLeft() const
    {
        return mSkill.isLeft;
    }
    int                getSkillId()
    {
        int skill;
        if (isLeft()) {
            skill = D2Util::getLeftSkillId();
        } else {
            skill = D2Util::getRightSkillId();
        }

        return skill;
    }

    virtual void        start() override
    {
        DefSubclassProc(origD2Hwnd, WM_KEYDOWN, mSkill.key, 0);
        DefSubclassProc(origD2Hwnd, WM_KEYUP, mSkill.key, 0);

        int origId = getSkillId();
        trace("%s: Start skill %d, orig %d\n", __FUNCTION__, mSkill.skillId, origId);

        if (origId == mSkill.skillId) {
            fcDbg(L"no change required orig %d, cur %d", origId, mSkill.skillId);
            return sendMouseDown();
        }
        fcDbg(L"Set Skill to %d cur %d", mSkill.skillId, origId);
        D2Util::setSkill(mSkill.skillId, isLeft());
        next(&RunTask::preCheck, 5);
    }

    void                preCheck()
    {
        mTime.start();
        checkSkill();
    }

    void                checkSkill()
    {
        if (!isFastCastAble()) {
            return done();
        }

        int skillId = getSkillId();

        trace("%s: wait skill %d, cur %d\n", __FUNCTION__, mSkill.skillId, skillId);
        if (skillId != mSkill.skillId) {
            if (mTime.elapsed() >= 250) {
                log_verbose("Failed to set skill id to %d, cur %d\n", mSkill.skillId, skillId);
                return done();
            }
            return next(&RunTask::checkSkill, 5);
        }
        fcDbg(L"Skill %d set", mSkill.skillId);
        trace("%s: wait skill %d done\n", __FUNCTION__, mSkill.skillId);
        //next(&RunActor::sendMouseDown, 0);
        sendMouseDown();
    }

    void sendMouseDown()
    {
        if (!D2Util::isGameScreen()) {
            return done();
        }

        POINT screenPos = { MOUSE_POS->x, MOUSE_POS->y};
        mCurrentPos = screenPos;
        //D2Util::screenToAutoMap(&screenPos, &mCurrentPos);
        trace("%s: send mouse event skill %d cur %d\n", __FUNCTION__,
              mSkill.skillId, getSkillId());
        DWORD pos = ((DWORD)mCurrentPos.x) | (((DWORD)mCurrentPos.y) << 16);
        if (isLeft()) {
            int         holdKey = -1;

            if (fastCastLeftHoldSkills.find(mSkill.skillId) != fastCastLeftHoldSkills.end()) {
                holdKey = D2Util::getHoldKey();
                fcDbg(L"Hold key: %d", holdKey);
            }

            fcDbg(L"Hold key: %d, skill %d", holdKey, mSkill.skillId);
            if (holdKey != -1) {
                DefSubclassProc(origD2Hwnd, WM_KEYDOWN, holdKey, 0);
            }

            mySendMessage(origD2Hwnd, WM_LBUTTONDOWN, MK_LBUTTON, pos);
            mySendMessage(origD2Hwnd, WM_LBUTTONUP, MK_LBUTTON, pos);

            if (holdKey != -1) {
                DefSubclassProc(origD2Hwnd, WM_KEYUP, holdKey, 0);
            }
        } else {
            mySendMessage(origD2Hwnd, WM_RBUTTONDOWN, MK_RBUTTON, pos);
            mySendMessage(origD2Hwnd, WM_RBUTTONUP, MK_RBUTTON, pos);
        }
        next(&RunTask::done, 10);
    }

    void sendMouseUp()
    {
        mySendMessage(origD2Hwnd, WM_RBUTTONUP, MK_RBUTTON,
                      ((DWORD)mCurrentPos.x) | (((DWORD)mCurrentPos.y) << 16));
        next(&RunTask::done, 5);
        trace("%s: skill %d mouse sent\n", __FUNCTION__, mSkill.skillId);
    }

    void done()
    {
        complete();
    }

private:
    ElapsedTime mTime;
    SkillTime   mSkill;
    POINT       mCurrentPos;
};

class RestoreTask: public Task {
public:
    RestoreTask(bool isLeft):
        mIsLeft(isLeft), mOrigSkillId(-1), mCurrentSwitch(0xff)
    {
    }

    void                clear()
    {
        cancel();
        mOrigSkillId = -1;
    }

    void                save()
    {
        auto weaponSwitch = D2Util::getWeaponSwitch();

        cancel();
        if (mOrigSkillId == -1 || mCurrentSwitch != weaponSwitch) {
            mOrigSkillId = getSkillId();
            mCurrentSwitch = weaponSwitch;
            trace("Left %u save skill to %u switch %u", mIsLeft, mOrigSkillId, mCurrentSwitch);
        }
    }

    void                resume()
    {
        trace("Left %u resume to restore skill %u", mIsLeft, mOrigSkillId);
        run();
    }

private:
    int                 getSkillId()
    {
        if (mIsLeft) {
            return D2Util::getLeftSkillId();
        } else {
            return D2Util::getRightSkillId();
        }
    }

    void                setSkillId(int skillId)
    {
        D2Util::setSkill(mOrigSkillId, mIsLeft);
    }

    bool                needAbort() const
    {
        if (mOrigSkillId == -1) {
            return true;
        }

        if (D2Util::getWeaponSwitch() != mCurrentSwitch) {
            fcDbg(L"Switch weapon, finish restore");
            return true;
        }

        // User is selecting a new skill or switching weapon
        if (D2Util::uiIsSet(UIVAR_CURRSKILL)) {
            return true;
        }

        return false;
    }

    virtual void        start() override
    {
        trace("Restore: left %u orig %d switch %d cur %d\n",
              mIsLeft, mOrigSkillId, mCurrentSwitch, getSkillId());
        if (needAbort()) {
            return finishRestore();
        }
        next(&RestoreTask::startRestore, 50);
    }

    void                startRestore()
    {
        mTime.start();
        return sendRestore();
    }

    void                sendRestore()
    {
        if (needAbort()) {
            return finishRestore();
        }

        if (mTime.elapsed() >= 30000) {
            // give up after 30 seconds
            trace("Restore failed after 30 seconds");
            return finishRestore();
        }

        int skillId = getSkillId();
        if (skillId != mOrigSkillId) {
            // Retry
            trace("sendRestore: set kill to %d cur %d\n", mOrigSkillId, getSkillId());
            setSkillId(mOrigSkillId);
            next(&RestoreTask::sendRestore, 10);
            return;
        }

        if (mIsLeft) {
            return finishRestore();
        }

        // Fix SK triggered by hackmap
        // This possible happens in 3 seconds.
        // Still some very corner case that fails to restore, but good enough now.
        if (mTime.elapsed() <= 30000) {
            next(&RestoreTask::sendRestore, 50);
            return;
        }
        finishRestore();
    }

    void                finishRestore()
    {
        mOrigSkillId = -1;
        done();
    }

    void                done()
    {
        complete();
    }

private:
    bool                mIsLeft;
    int                 mOrigSkillId;
    BYTE                mCurrentSwitch;
    ElapsedTime         mTime;
};

class FastCastActor {
    enum State {
        Idle,
        Skill,
        Restore,
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

    FastCastActor(): mState(Idle), mCrankCount(0),
                     mRestoreTasks{Right, Left}
    {
        auto f = [this](Event::Type) { stop(); };
        Event::add(Event::GameStart, f);
        Event::add(Event::GameEnd, f);

        mRunSkillTask.setComplete([this](Task *) {
            crank();
        });
    }

    // call when game exit
    void  stop()
    {
        mRunSkillTask.cancel();
        for (auto& task: mRestoreTasks) {
            task.clear();
        }

        mState = Idle;
        mCrankCount = 0;
        mPendingSkills.clear();
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
    void crank()
    {
        int cur = mCrankCount;

        mCrankCount += 1;
        if (cur != 0) {
            trace("Crank: cur %d skip");
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

        startNextSkill();
        return Wait;
    }

    Result startNextSkill()
    {
        if (!D2Util::isGameScreen()) {
            mPendingSkills.clear();
            mState = Restore;
            return Continue;
        }

        mState = Skill;
        runNextSkill();
        return Wait;
    }

    Result handleSkill()
    {
        trace("SKILL: running %d empty %d\n", mRunSkillTask.isRunning(), mPendingSkills.empty());
        if (mRunSkillTask.isRunning()) {
            return Wait;
        }

        if (!mPendingSkills.empty()) {
            return startNextSkill();
        }

        mState = Restore;
        return Continue;
    }

    Result handleRestore()
    {
        // mRightRestore.start();
        // mLeftRestore.start();
        bool    needRestore = true;

        // optimization
        if (!mPendingSkills.empty()) {
            auto & st = mPendingSkills.front();
            if (st.isLeft == mCurrentSkill.isLeft) {
                needRestore = false;
            }
        }

        if (needRestore) {
            restoreSkill();
        }

        if (!mPendingSkills.empty()) {
            return startNextSkill();
        }

        mState = Idle;
        return Continue;
    }

    void runNextSkill()
    {
        mCurrentSkill = mPendingSkills.front();
        mPendingSkills.pop_front();

        saveSkill();
        mRunSkillTask.startSkill(mCurrentSkill);
    }

    int getRestoreIndex()
    {
        return mCurrentSkill.isLeft? Left : Right;
    }

    void saveSkill()
    {
        mRestoreTasks[getRestoreIndex()].save();
    }

    void restoreSkill()
    {
        mRestoreTasks[getRestoreIndex()].resume();
    }

private:
    enum {
        Right,
        Left,
    };

    State               mState;
    int                 mCrankCount;

    std::list<SkillTime> mPendingSkills;
    RunTask             mRunSkillTask;

    SkillTime           mCurrentSkill;

    RestoreTask         mRestoreTasks[2];
};

static bool doFastCast(BYTE key, BYTE repeat)
{
    if (!isFastCastAble()) {
        D2Util::showVerbose(L"Not in Game: %d, repeat %d", D2CheckUiStatus(0), repeat);
        return false;
    }

    trace("key: %d, repeat %d, swap 0x%x", key, repeat, WEAPON_SWITCH);
    int         func = D2Util::getKeyFunc(key);

    if (func < 0) {
        return false;
    }

    bool        isLeft;
    int         skillId = D2Util::getSkillId(func, &isLeft);
    if (skillId < 0) {
        fcDbg(L"KEY %u -> %u (unbind)", key, func);
        return false;
    }

    fcDbg(L"KEY %u -> %u -> skill %u, left: %d\n", key, func, skillId, isLeft);

    FastCastActor::inst().startSkill({key, skillId, isLeft, GetTickCount64()});

    return true;
}

static bool fastCastToggle(struct HotkeyConfig * config)
{
    if (fastCastEnabled) {
        FastCastActor::inst().stop();
        fastCastEnabled = false;
    } else {
        fastCastEnabled = true;
    }
    log_verbose("FastCast: toggle: %d\n", fastCastEnabled);
    D2Util::showInfo(L"快速施法已%s", fastCastEnabled? L"啟用" : L"禁用");

    if ((config->hotKey & (Hotkey::Ctrl | Hotkey::Alt)) != 0) {
        return true;
    }

    return false;
}

static void fastCastLoadConfig()
{
    auto section = CfgLoad::section("helper.fastcast");

    fastCastDebug = section.loadBool("debug", false);
    fastCastEnabled = section.loadBool("enable", true);
    auto keyString = section.loadString("toggleKey");
    fastCastToggleKey = Hotkey::parseKey(keyString);

    std::vector<std::string>    holdSkills;
    section.loadList("leftHold", holdSkills);
    log_verbose("left hold skills %zu", holdSkills.size());
    if (holdSkills.empty()) {
        // Enabl TP Hold by default
        fastCastLeftHoldSkills.insert(54);
    }
    for (auto& s: holdSkills) {
        bool ok;
        int skillId = helper::toInt(s, &ok);
        log_verbose("skill %s -> %d, ok %u", s.c_str(), skillId, ok);
        if (ok && skillId > 0) {
            fastCastLeftHoldSkills.insert(skillId);
        }
    }

    if (fastCastToggleKey != Hotkey::Invalid) {
        log_trace("FastCast: toggle key %s -> 0x%llx\n", keyString.c_str(), (unsigned long long)fastCastToggleKey);
    }
    log_verbose("FastCast: enable: %d, debug %d\n", fastCastEnabled, fastCastDebug);
}

static int fastCastModuleInstall()
{
    fastCastLoadConfig();
    keyRegisterHandler(doFastCast);
    FastCastActor::inst();

    if (fastCastToggleKey != Hotkey::Invalid) {
        keyRegisterHotkey(fastCastToggleKey, fastCastToggle, nullptr);
    }

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
