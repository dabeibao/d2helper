#ifndef HELPFUNC_H
#define HELPFUNC_H

#include <windows.h>
#include <stdint.h>

#define INST_NOP 0x90
#define INST_CALL 0xe8
#define INST_JMP 0xe9
#define INST_JMPR 0xeb
#define INST_RET 0xc3

extern char szPluginPath[MAX_PATH];
DWORD GetDllOffset(const char *dll, int offset);
DWORD GetDllOffset(int num);
DWORD ReadLocalDWORD(DWORD pAddress);
void WriteLocalBYTES(DWORD pAddress, const void *buf, int len);
void ReadLocalBYTES(DWORD pAddress, void *buf, int len);


HANDLE OpenFileNew(char *filename);
HANDLE OpenFileRead(char *filename);
HANDLE OpenFileWrite(char *filename);
DWORD WriteFile(HANDLE hFile, void *buf, DWORD len);
DWORD ReadFile(HANDLE hFile, void *buf, DWORD len);
char *ReadFileLine(char *str, int len, HANDLE hFile);
BYTE *AllocReadFile(char *filename);

wchar_t * __cdecl wsprintfW2(wchar_t *dest, char *fmt, ...);
void SwapInt(int &num1, int &num2);
void *memcpy2(void *dest, const void *src, size_t count);
void wcscpy2(wchar_t *dest, char *src);
wchar_t * wcsrcat(wchar_t *dest, wchar_t *src);
void trimspaces(char *str);
void trimspaces(wchar_t *str);
char *DblToStr(double dblSour,int decimal,char *desc);

char *AllocStrN(char *str, int len);

wchar_t * __cdecl wsprintfW2Colour(wchar_t *dest, int col, char *fmt, ...);
wchar_t * __fastcall ColourD2String(wchar_t *str, DWORD col);
wchar_t * wscolorcpy(wchar_t *dest, wchar_t *src , BYTE color);
void ConvertToColorString( BYTE *dst , char  *src , int size);

void PatchCALL(DWORD addr, DWORD param, DWORD len);
void PatchJMP(DWORD addr, DWORD param, DWORD len);
void PatchFILL(DWORD addr, DWORD param, DWORD len);
void PatchVALUE(DWORD addr, DWORD param, DWORD len);
void * PatchDetour(DWORD addr, DWORD param, DWORD len);
int PatchCompare(DWORD addr, const uint8_t * old, int oldSize, const uint8_t * newCode, int newSize);;

int MyMultiByteToWideChar(
						UINT CodePage,         // code page
						DWORD dwFlags,         // character-type options
						LPCSTR lpMultiByteStr, // string to map
						int cbMultiByte,       // number of bytes in string
						LPWSTR lpWideCharStr,  // wide-character buffer
						int cchWideChar        // size of buffer
						);
	void GB2GBK(char *szBuf);



#endif
