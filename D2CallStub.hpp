#pragma once

#include <windows.h>

void __fastcall D2SetAutomapParty(DWORD flag);
void __fastcall D2SetAutomapNames(DWORD flag);

DWORD __fastcall D2ChatInput(D2MSG *pmsg);

DWORD __fastcall D2CheckUiStatus(DWORD dwUiNo);

AutomapLayer* __fastcall D2InitAutomapLayer(DWORD nLayerNo);

UniqueItemTxt* __fastcall D2GetUniqueItemTxt(DWORD dwIdx);

ItemStatCostTxt* __fastcall D2GetItemStatCostTxt(DWORD dwStatNo);

PropertiesTxt* __fastcall D2GetPropertiesTxt(DWORD dwPropNo);

SetItemTxt* __fastcall D2GetSetItemTxt(DWORD dwIdx);

ItemTypeTxt* __fastcall D2GetItemTypeTxt(DWORD dwIdx);

RuneWordTxt* __fastcall D2GetRuneWordTxt(DWORD dwIdx);

DWORD __fastcall D2GetRuneWordTxtIndex(DWORD dwFileNo);

CellFile* __fastcall D2LoadUiImage(char* szPath);
