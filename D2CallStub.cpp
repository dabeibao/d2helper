#include <windows.h>
#include "d2ptrs.h"

void __declspec(naked) __fastcall D2SetAutomapParty(DWORD flag)
{
	__asm {
		push esi
		mov esi, ecx
		call D2SetAutomapPartyStub
		pop esi
		ret
	}
}

void __declspec(naked) __fastcall D2SetAutomapNames(DWORD flag)
{
	__asm {
		push esi
		mov esi, ecx
		call D2SetAutomapNamesStub
		pop esi
		ret
	}
}

//ÁÄÌìÏûÏ¢
DWORD __declspec(naked) __fastcall D2ChatInput(D2MSG *pmsg)
{
	__asm {
		mov eax, 0xF
		push ecx
		call D2ChatInputStub
		ret
	}
}

DWORD __declspec(naked) __fastcall D2CheckUiStatus(DWORD dwUiNo)
{
	__asm {
		mov eax, ecx
		call D2CheckUiStatusStub
		ret
	}
}

__declspec(naked) AutomapLayer* __fastcall D2InitAutomapLayer(DWORD nLayerNo)
{
	__asm {
		mov eax, ecx
		call D2InitAutomapLayerStub
		ret
	}
}


__declspec(naked) UniqueItemTxt*  __fastcall D2GetUniqueItemTxt(DWORD dwIdx)
{
	__asm {
		mov eax, ecx
		mov ecx, [p_D2DataTables]
		mov ecx , [ecx]
		cmp eax, dword ptr [ecx+0xC28]
		jge unkret
		mov edx, dword ptr [ecx+0xC24]
		imul eax, 0x14C
		add eax, edx
		ret
unkret:
		xor eax , eax
		ret
	}
}

__declspec(naked) ItemStatCostTxt* __fastcall D2GetItemStatCostTxt(DWORD dwStatNo)
{
	__asm {
		mov eax, ecx
		mov ecx, [p_D2DataTables]
		mov ecx , [ecx]
		cmp eax, dword ptr [ecx+0xBD4]
		jge unkret
		mov edx, dword ptr [ecx+0xBCC]
		imul eax, 0x144
		add eax, edx
		ret
unkret:
		xor eax , eax
		ret
	}
}


__declspec(naked) PropertiesTxt* __fastcall D2GetPropertiesTxt(DWORD dwPropNo)
{
	__asm {
		mov eax, ecx
		mov ecx, [p_D2DataTables]
		mov ecx , [ecx]
		cmp eax, dword ptr [ecx+0xAC]
		jge unkret
		mov edx, dword ptr [ecx+0xA4]
		imul eax, 0x2E
		add eax, edx
		ret
unkret:
		xor eax , eax
		ret
	}
}

__declspec(naked) SetItemTxt* __fastcall D2GetSetItemTxt(DWORD dwIdx)
{
	__asm {
			mov eax, ecx
			mov ecx, [p_D2DataTables]
			mov ecx , [ecx]
			cmp eax, dword ptr [ecx+0xC1C]
			jge unkret
			mov edx, dword ptr [ecx+0xC18]
			imul eax, 0x1B8
			add eax, edx
			ret
	unkret:
			xor eax , eax
			ret
	}
}

__declspec(naked) ItemTypeTxt* __fastcall D2GetItemTypeTxt(DWORD dwIdx)
{
	__asm {
			mov eax, ecx
			mov ecx, [p_D2DataTables]
			mov ecx , [ecx]
			cmp eax, dword ptr [ecx+0xBFC]
			jge unkret
			mov edx, dword ptr [ecx+0xBF8]
			imul eax, 0xE4
			add eax, edx
			ret
	unkret:
			xor eax , eax
			ret
	}
}

__declspec(naked) RuneWordTxt* __fastcall D2GetRuneWordTxt(DWORD dwIdx)
{
	__asm {
			mov eax, ecx

			mov ecx, [p_D2RuneWords]
			mov ecx , [ecx]

			cmp eax, ecx
			jge unkret
			mov edx ,[p_D2RuneWordTxt]
			mov edx ,[edx]
			imul eax, 0x120
			add eax, edx
			ret
	unkret:
			xor eax , eax
			ret
	}
}

DWORD __declspec(naked) __fastcall D2GetRuneWordTxtIndex(DWORD dwFileNo)
{
	__asm {
			push ebx
			push ebp
			mov ebx , ecx

			mov ecx, [p_D2RuneWords]
			mov ecx , [ecx]

			mov edx ,[p_D2RuneWordTxt]
			mov edx ,[edx]

			xor ebp ,ebp
myloop:
			mov eax , ebp
			cmp eax, ecx
			jge unkret
			imul eax, 0x120
			add eax, edx

			inc ebp
			cmp bx , word ptr [eax+0x82]
			jne myloop

			mov eax , ebp
			dec eax
			pop ebp
			pop ebx
			ret
	unkret:
			xor eax , eax
			pop ebp
			pop ebx
			ret
	}
}



__declspec(naked) CellFile* __fastcall D2LoadUiImage(char* szPath)
{
	__asm
	{
		mov eax, ecx
		push 0
		call vD2LoadUiImageFunc
		retn
	}
}
