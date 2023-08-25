#pragma once

#include <windows.h>
#include <utility>
#include "d2structs.h"
#include "d2ptrs.h"
#include "Define.h"

namespace D2Util {

extern bool gVerbose;
extern bool gDebug;

enum ClientState {
    ClientStateNull,
    ClientStateMenu,
    ClientStateInGame,
    ClientStateBusy
};

inline void setVerbose(bool isVerbose)
{
    gVerbose = isVerbose;
}
inline void setDebug(bool isDebug)
{
    gDebug = isDebug;
}

void showInfo(const LPCWSTR fmt, ...);

inline bool inGame()
{
    return PLAYER != nullptr;
}

inline void showDebug(const LPCWSTR fmt, auto&&... args)
{
    if (gDebug) {
        showInfo(fmt, std::forward<decltype(args)>(args)...);
    }
}

inline void showVerbose(const LPCWSTR fmt, auto&&... args)
{
    if (gVerbose) {
        showInfo(fmt, std::forward<decltype(args)>(args)...);
    }
}

inline bool uiIsSet(int ui)
{
    if (ui >= 0x26) {
        return 0;
    }
    return UI_STATUS_TABLE[ui] != 0;
}

inline int getRightSkillId()
{
    if (PLAYER && PLAYER->pSkill && PLAYER->pSkill->pRightSkill && PLAYER->pSkill->pRightSkill->pSkillInfo) {
        return (int)PLAYER->pSkill->pRightSkill->pSkillInfo->wSkillId;
    }

    return -1;
}

inline BYTE getWeaponSwitch()
{
    return WEAPON_SWITCH;
}

int getKeyFunc(BYTE key);
int getSkillId(int func);
void setSkill(WORD skillId, bool left = false);
void castSkill(int x, int y, bool left = false);
void screenToAutoMap(const POINT * screenPos, POINT * mapPos);

ClientState getState();

// In game screen
bool isGameScreen();


}
