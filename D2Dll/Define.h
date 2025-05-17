#ifndef DEFINE_H
#define DEFINE_H

#define LEVELNO dwCurrentLevel
#define ACTNO dwCurrentAct

#define PLAYER (*p_D2PlayerUnit)
#define PLAYERLIST (*p_D2RosterUnitList)
#define VIEWITEMUNIT (*p_D2CurrentViewItem)

#define GAMEINFO (*p_D2GameInfo)
#define ACT (*p_D2DrlgAct)
#define LAYER (*p_D2AutomapLayer)
#define LAYERLIST (*p_D2AutomapLayerList)

#define DIVISOR (*p_D2Divisor)
#define OFFSET (*p_D2Offset)
#define AUTOMAPPOS (*p_D2AutomapPos)
#define MINMAPOFFSET (*p_D2MinimapOffset)
#define MINMAPTYPE (*p_D2MinmapType)
#define SCREENSIZE (*p_D2ScreenSize)
#define INFOX (*p_D2InfoPositionX)-16
#define INFOY (*p_D2InfoPositionY)
#define DRAWOFFSET (*p_D2DrawOffset)

#define MAPSHAKEX (*p_D2MapShakeX)
#define MAPSHAKEY (*p_D2MapShakeY)

#define DIFFICULTY (*p_D2Difficulty)
#define EXPANSION (*p_D2Expansion)

#define PING (*p_D2Ping)
#define FPS (*p_D2Fps)

#define QUESTDATA (*p_D2QuestData)
#define QUESTPAGE (*p_D2QuestPage)
#define GAMEQUESTDATA (*p_D2GameQuestData)
#define FOCUSECONTROL (*p_D2FocusedControl)

#define KEY_TABLE (p_D2KeyTable)
#define FUNC_SKILL_TABLE (p_D2FuncSkillTable)
#define SKILL_HAND_TABLE (p_D2SkillHandTable)

#define UI_STATUS_TABLE (p_D2UiStatusTable)

#define UI_PANEL_STATE (*p_D2UIPanelState)

#define MOUSE_POS       (p_D2MousePos)
#define MOUSE_OFFSET_X  (*p_D2MouseOffsetX)
#define MOUSE_OFFSET_Y  (*p_D2MouseOffsetY)

#define WEAPON_SWITCH   (*p_D2WeaponSwitch)


#endif

