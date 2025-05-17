#include <windows.h>
#include <stdio.h>
#include "d2ptrs.h"
#include "D2Utils.hpp"
#include "Define.h"
#include "d2structs.h"
#include "d2vars.h"

bool D2Util::gVerbose;
bool D2Util::gDebug;

void D2Util::showInfo(const LPCWSTR fmt, ...)
{
    wchar_t     info[512];
    va_list     ap;

    if (!inGame()) {
        return;
    }
    va_start(ap, fmt);
    _vsnwprintf(info, sizeof(info) / sizeof(info[0]), fmt, ap);
    va_end(ap);
    D2ShowGameMessage(info, 0);
}

int D2Util::getKeyFunc(BYTE key)
{
    int         index = 0;

    for (int i = 0; i < D2_KEY_TABLE_FIRST_FUNC_NUM; i += 1, index += 1) {
        int     offset = D2_KEY_TABLE_FIRST_FUNC + i;
        auto    keyMap = &KEY_TABLE[offset];
        if (keyMap->key == 0xffff) {
            continue;
        }
        if (keyMap->key == key) {
            return index / 2;
        }
    }

    for (int i = 0; i < D2_KEY_TABLE_SECOND_FUNC_NUM; i += 1, index += 1) {
        int     offset = D2_KEY_TABLE_SECOND_FUNC + i;
        auto    keyMap = &KEY_TABLE[offset];
        if (keyMap->key == 0xffff) {
            continue;
        }
        if (keyMap->key == key) {
            return index / 2;
        }
    }

    return -1;
}

int D2Util::getSkillId(int func, bool * isLeft)
{
    auto        table = FUNC_SKILL_TABLE;
    DWORD       id = table[func].skillId;

    if (id >= 0xffff) {
        return -1;
    }

    auto        handTable = SKILL_HAND_TABLE;
    *isLeft = handTable[func].isLeft != 0;

    return (int)id;
}

void D2Util::setSkill(WORD skillId, bool left)
{
    BYTE        packet[9] = {0x3c,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF};

    *(WORD *)&packet[1] = skillId;
    if (left) {
        packet[4] = 0x80;
    }

    D2SendPacket(sizeof(packet), 0, packet);

}

void D2Util::setAndUpdateSkill(WORD skillId, bool isLeft)
{
    if (isLeft) {
        D2SetLeftActiveSkill(PLAYER, skillId, 0xffffffff);
    } else {
        D2SetRightActiveSkill(PLAYER, skillId, 0xffffffff);
    }

    D2Util::setSkill(skillId, isLeft);
}

void D2Util::castSkill(int x, int y, bool left)
{
    BYTE        packet[5];

    packet[0] = left? 0x05 : 0x0c;
    *(WORD *)&packet[1] = x;
    *(WORD *)&packet[3] = y;
    D2SendPacket(sizeof(packet), 0, packet);
}

void D2Util::screenToAutoMap(const POINT * screenPos, POINT * mapPos)
{
    LONG        x = screenPos->x + MOUSE_OFFSET_X;
    LONG        y = screenPos->y + MOUSE_OFFSET_Y;

    D2AbsScreenToMap(&x, &y);
    mapPos->x = x;
    mapPos->y = y;
}

D2Util::ClientState D2Util::getState()
{
    auto firstControl = *p_D2FirstControl;
    UnitAny * player = PLAYER;

    if (player == nullptr) {
        if (firstControl != nullptr) {
            return ClientStateMenu;
        }
        return ClientStateNull;
    }
    if (player->pUpdateUnit != nullptr) {
        return ClientStateBusy;
    }
    if (player->pInventory && player->pMonPath &&
        player->pMonPath->pRoom1 && player->pMonPath->pRoom1->pRoom2 && player->pMonPath->pRoom1->pRoom2->pDrlgLevel &&
        player->pMonPath->pRoom1->pRoom2->pDrlgLevel->dwLevelNo) {
        return ClientStateInGame;
    }

    return ClientStateBusy;
}

bool D2Util::isGameScreen()
{
    if (D2Util::getState() != D2Util::ClientStateInGame) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_CURRSKILL)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_CHATINPUT)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_INTERACT)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_GAMEMENU)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_CFGCTRLS)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_NPCTRADE)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_MODITEM)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_MSGLOG)) {
        return false;
    }

    if ((UI_PANEL_STATE & UiPanelOpenBoth) == UiPanelOpenBoth) {
        return false;
    }

#if 0
    if (D2Util::uiIsSet(UIVAR_PPLTRADE)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_STASH)) {
        return false;
    }
    if (D2Util::uiIsSet(UIVAR_CUBE)) {
        return false;
    }
    bool left = D2Util::uiIsSet(UIVAR_STATS) || D2Util::uiIsSet(UIVAR_QUEST) || D2Util::uiIsSet(UIVAR_PET);
    bool right = D2Util::uiIsSet(UIVAR_INVENTORY) || D2Util::uiIsSet(UIVAR_SKILLS);
    if (left && right) {
        return false;
    }
#endif
    return true;
}

