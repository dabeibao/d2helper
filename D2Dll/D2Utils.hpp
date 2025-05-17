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

inline int getLeftSkillId()
{
    if (PLAYER && PLAYER->pSkill && PLAYER->pSkill->pLeftSkill && PLAYER->pSkill->pLeftSkill->pSkillInfo) {
        return (int)PLAYER->pSkill->pLeftSkill->pSkillInfo->wSkillId;
    }

    return -1;
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

int getKeyFuncFromTable(BYTE key, D2_KEY_TABLE *keyTable);
int getKeyFunc(BYTE key);

inline int getHoldKey()
{
    for (auto i = 0; i < 2; i += 1) {
        auto  keyMap = &KEY_TABLE[D2_KEY_HOLD_OFFSET + i];
        if (keyMap->key != 0xffff) {
            return keyMap->key;
        }
    }

    return -1;
}

inline void activeItem(UnitAny * item, WORD x, WORD y)
{
    BYTE    packet[13] = { 0x20,};

    *(DWORD *)&packet[1] = item->dwUnitId;
    *(WORD *)&packet[5] = x;
    *(WORD *)&packet[9] = y;
    D2SendPacket(sizeof(packet), 0, packet);
}

int getSkillId(int func, bool * isLeft);
void setSkill(WORD skillId, bool left = false);
void setAndUpdateSkill(WORD skillId, bool isLeft);
void castSkill(int x, int y, bool left = false);
void screenToAutoMap(const POINT * screenPos, POINT * mapPos);

enum ClickType {
    LeftDown = 0,
    LeftUp = 2,
    RightDown = 3,
    RightUp = 5,
};

enum {
    ClickShift = 0x04,
    ClickRun = 0x08,
};

inline void sendClick(ClickType clickType, int screenX, int screenY, bool isShift)
{
    int flags = isShift? 0x04 : 0;
    if (*p_D2AlwaysRun) {
        flags |= 0x08;
    }
    D2ClickMap(clickType, screenX, screenY, flags);
}

ClientState getState();

// In game screen
bool isGameScreen();

}
