#include <windows.h>
#include <commctrl.h>
#include <list>
#include <set>
#include "HelpFunc.h"
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
#define trace(fmt, ...)         do { if (fastCastDebug) log_trace("%s:" fmt, __FUNCTION__, ##__VA_ARGS__); log_flush(); } while (0)

static bool fastCastDebug;
static bool fastCastEnabled;
static uint32_t fastCastToggleKey = Hotkey::Invalid;

static std::set<int> fastCastAttackSkills = {
    0,
    // amazon
    10, 14, 19, 24, 30, 34,

    // sor

    // nec

    // pal
    96, 97, 106, 107,

    // Bar
    131, 142, 126, 133, 139, 144, 147, 151, 152,

    // Dru

    // Asn
    254, 255, 260, 259, 265, 270, 269, 247, 275, 280,
};

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
        int origId = getSkillId();
        trace("Start skill %d, orig %d\n", mSkill.skillId, origId);
        mOrigSkill = origId;

        // For the map
        DefSubclassProc(origD2Hwnd, WM_KEYDOWN, mSkill.key, 0);
        DefSubclassProc(origD2Hwnd, WM_KEYUP, mSkill.key, 0);

        D2Util::setAndUpdateSkill(mSkill.skillId, isLeft());
        fcDbg(L"Set Skill to %d cur %d", mSkill.skillId, origId);
        //next(&RunTask::preCheck, 5);
        preCheck();
    }

    void                preCheck()
    {
        trace("start wait time");
        mTime.start();
        checkSkill();
    }

    void                checkSkill()
    {
        if (!isFastCastAble()) {
            return done();
        }

        int skillId = getSkillId();

        trace("wait skill %d, cur %d\n", mSkill.skillId, skillId);
        if (skillId != mSkill.skillId) {
            if (mTime.elapsed() >= 250) {
                log_verbose("Failed to set skill id to %d, cur %d\n", mSkill.skillId, skillId);
                return done();
            }
            return next(&RunTask::checkSkill, 1);
        }
        fcDbg(L"Skill %d set", mSkill.skillId);
        trace("wait skill %d done\n", mSkill.skillId);
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
        trace("send mouse event skill %d cur %d\n",
              mSkill.skillId, getSkillId());
        DWORD pos = ((DWORD)mCurrentPos.x) | (((DWORD)mCurrentPos.y) << 16);
        if (isLeft()) {
            int         holdKey = -1;

            // It is not an attack skill
            if (fastCastAttackSkills.find(mSkill.skillId) == fastCastAttackSkills.end()) {
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
        done();
    }

    void done()
    {
        complete();
    }

private:
    ElapsedTime mTime;
    SkillTime   mSkill;
    POINT       mCurrentPos;
    int         mOrigSkill;
};

class RestoreTask: public Task {
public:
    RestoreTask(bool isLeft):
        mIsLeft(isLeft), mIsMonitoring(false), mOrigSkillId(-1), mCurrentSwitch(0xff)
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

    void                onServerSkillUpdate(int nSkillId)
    {
        if (nSkillId == mOrigSkillId) {
            return;
        }
        if (isRunning() && mIsMonitoring) {
            mTimer.stop();
            execute();
        }
    }

private:
    virtual void        stop() override
    {
        Task::stop();
        mIsMonitoring = false;
    }

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
        D2Util::setAndUpdateSkill(skillId, mIsLeft);
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
            trace("Selecting CURR skill, abort\n");
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
        //next(&RestoreTask::startRestore, 50);
        startRestore();
    }

    void                startRestore()
    {
        mTime.start();
        mIsMonitoring = true;
        return sendRestore();
    }

    void                sendRestore()
    {
        if (needAbort()) {
            return finishRestore();
        }

        if (mTime.elapsed() >= 60000) {
            // give up after 60 seconds
            trace("Restore failed after 60 seconds");
            return finishRestore();
        }

        int skillId = getSkillId();
        if (skillId != mOrigSkillId) {
            // Retry
            trace("sendRestore: set kill to %d cur %d\n", mOrigSkillId, getSkillId());
            setSkillId(mOrigSkillId);
            next(&RestoreTask::sendRestore, 1);
            return;
        }

        if (mTime.elapsed() <= 3000) {
            next(&RestoreTask::sendRestore, 10);
            return;
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
        mIsMonitoring = false;
        mOrigSkillId = -1;
        done();
    }

    void                done()
    {
        complete();
    }

private:
    bool                mIsLeft;
    bool                mIsMonitoring;
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

    void onServerSkillUpdate(bool isLeft, int nSkillId)
    {
        trace("Server set skill, left %d, skill %d\n", isLeft, nSkillId);
        if (isLeft) {
            mRestoreTasks[Left].onServerSkillUpdate(nSkillId);
        } else {
            mRestoreTasks[Right].onServerSkillUpdate(nSkillId);
        }
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
            trace("Crank: cur %d skip", cur);
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

        mState = Restore;
        return Continue;
    }

    Result handleRestore()
    {
        // mRightRestore.start();
        // mLeftRestore.start();
        bool    needRestore = true;

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

static std::vector<int> fastCastLoadSkillList(Config::Section& section, const char * key)
{
    std::vector<std::string>    list;
    std::vector<int>            skills;

    section.loadList(key, list);
    for (auto& s: list) {
        bool ok;
        int skillId = helper::toInt(s, &ok);
        log_verbose("%s %s -> %d, ok %u", key, s.c_str(), skillId, ok);
        if (!ok) {
            continue;
        }
        if (skillId < 0) {
            continue;
        }
        skills.push_back(skillId);
    }
    return skills;
}

static void fastCastLoadConfig()
{
    auto section = CfgLoad::section("helper.fastcast");

    fastCastDebug = section.loadBool("debug", false);
    fastCastEnabled = section.loadBool("enable", true);
    auto keyString = section.loadString("toggleKey");
    fastCastToggleKey = Hotkey::parseKey(keyString);

    auto attackSkills = fastCastLoadSkillList(section, "attackSkills");
    log_verbose("attack skills %zu", attackSkills.size());
    for (auto skillId: attackSkills) {
        fastCastAttackSkills.insert(skillId);
    }

    auto castSkills = fastCastLoadSkillList(section, "castSkills");
    log_verbose("case skills %zu", castSkills.size());
    for (auto skillId: castSkills) {
        fastCastAttackSkills.erase(skillId);
    }


    if (fastCastToggleKey != Hotkey::Invalid) {
        log_trace("FastCast: toggle key %s -> 0x%llx\n", keyString.c_str(), (unsigned long long)fastCastToggleKey);
    }
    log_verbose("FastCast: enable: %d, debug %d\n", fastCastEnabled, fastCastDebug);
}

static void __stdcall setLeftActiveSkillHook(UnitAny * unit, int nSkillId, DWORD ownerGUID)
{

    D2SetLeftActiveSkill(unit, nSkillId, ownerGUID);
    FastCastActor::inst().onServerSkillUpdate(true, nSkillId);
}

static void __stdcall setRightActiveSkillHook(UnitAny * unit, int nSkillId, DWORD ownerGUID)
{
    D2SetRightActiveSkill(unit, nSkillId, ownerGUID);
    FastCastActor::inst().onServerSkillUpdate(false, nSkillId);
}

static void patch()
{
    // CPU Disasm
    // Address   Hex dump          Command                                                              Comments
    // 6FB5C7E0  |.  E8 29FDF5FF   CALL <JMP.&D2Common.#10546>                                          ; |\D2Common.#10546
    // 6FB5C7E5  |.  5F            POP EDI                                                              ; |
    // 6FB5C7E6  |.  5E            POP ESI                                                              ; |
    // 6FB5C7E7  |.  C3            RETN                                                                 ; |
    // base = D2Client(0x6FAB0000) offset = 0x6FB5C7E0 - base = 0xAC7E0
    DWORD offset = GetDllOffset("D2CLIENT.DLL", 0x6FB5C7E0 - DLLBASE_D2CLIENT);
    PatchCALL(offset, (DWORD)(uintptr_t)setLeftActiveSkillHook, 5);

    // CPU Disasm
    // Address   Hex dump          Command                                                              Comments
    // 6FB5C7ED  |.  E8 3AFDF5FF   CALL <JMP.&D2Common.#10978>                                          ; \D2Common.#10978
    // 6FB5C7F2  |>  5F            POP EDI
    // 6FB5C7F3  |.  5E            POP ESI
    // 6FB5C7F4  \.  C3            RETN
    // base = D2Client(0x6FAB0000) offset = 0x6FB5C7ED - base = 0xAC7ED
    offset = GetDllOffset("D2CLIENT.DLL", 0x6FB5C7ED - DLLBASE_D2CLIENT);
    PatchCALL(offset, (DWORD)(uintptr_t)setRightActiveSkillHook, 5);
}

static int fastCastModuleInstall()
{
    fastCastLoadConfig();
    keyRegisterHandler(doFastCast);
    FastCastActor::inst();

    if (fastCastToggleKey != Hotkey::Invalid) {
        keyRegisterHotkey(fastCastToggleKey, fastCastToggle, nullptr);
    }

    patch();

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
