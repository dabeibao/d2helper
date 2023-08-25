#ifndef D2_PTR_H
#define D2_PTR_H

#include <windows.h>
#include "d2structs.h"

typedef enum D2Version {
  D2_VER_113C,
  D2_VER_113D,

  UNKNOWN = -1,
} eGameVersion;

enum DllBase { //为查找反汇编方便,
               //以全地址方式定义函数地址，所以需要注意此处基值与反汇编文件中对应
    DLLBASE_D2CLIENT = 0x6FAB0000,
    DLLBASE_D2COMMON = 0x6FD50000,
    DLLBASE_D2GFX = 0x6FA80000,
    DLLBASE_D2WIN = 0x6F8E0000,
    DLLBASE_D2LANG = 0x6FC00000,
    DLLBASE_D2CMP = 0x6FE10000,
    DLLBASE_D2MULTI = 0x6F9D0000,
    DLLBASE_BNCLIENT = 0x6FF20000,
    DLLBASE_D2NET = 0x6FBF0000,
    DLLBASE_STORM = 0x6FFB0000, // 6FBF0000  , 与D2NET冲突，修改值
    DLLBASE_FOG = 0x6FF50000,
    DLLBASE_D2GAME = 0x6FC20000,
    DLLBASE_D2LAUNCH = 0x6FA40000,
    DLLBASE_D2MCPCLIENT = 0x6FA20000
};

enum DllNo {
    DLLNO_D2CLIENT,
    DLLNO_D2COMMON,
    DLLNO_D2GFX,
    DLLNO_D2WIN,
    DLLNO_D2LANG,
    DLLNO_D2CMP,
    DLLNO_D2MULTI,
    DLLNO_BNCLIENT,
    DLLNO_D2NET,
    DLLNO_STORM,
    DLLNO_FOG,
    DLLNO_D2GAME,
    DLLNO_D2LAUNCH,
    DLLNO_D2MCPCLIENT
};

//#define DLLOFFSET(a1,b1) ((DLLNO_##a1)|(( ((b1)<0)?(b1):((b1)-DLLBASE_##a1)
//)<<8))
extern DWORD CalcFuncOffset_X(DWORD dwNo, DWORD dwBase, int iValue);
extern DWORD Map_D2Version();
#define DLLOFFSET(a1, b1) CalcFuncOffset_X(DLLNO_##a1, DLLBASE_##a1, b1)
#define DLLOFFSET2(a1, b1, b2)                                                 \
  ((D2_VER_113D == Map_D2Version())                                            \
       ? (CalcFuncOffset_X(DLLNO_##a1, DLLBASE_##a1, b2))                      \
       : (CalcFuncOffset_X(DLLNO_##a1, DLLBASE_##a1, b1)))

#define D2FUNCPTR(d1, o1, v1, t1, t2)                                           \
    typedef t1 D2##v1##_t t2;                                                   \
    extern D2##v1##_t *D2##v1;

#define D2FUNCPTR2(d1, o1, o2, v1, t1, t2)                              \
    typedef t1 D2##v1##_t t2;                                           \
    extern D2##v1##_t *D2##v1;

#define D2VARPTR(d1, o1, v1, t1)                                        \
    typedef t1 p_D2##v1##_t;                                            \
    extern p_D2##v1##_t *p_D2##v1;

#define D2VARPTR2(d1, o1, o2, v1, t1)                                   \
    typedef t1 p_D2##v1##_t;                                            \
    extern p_D2##v1##_t *p_D2##v1;

#define D2ASMPTR(d1, o1, v1) extern volatile DWORD vD2##v1;

#define D2ASMPTR2(d1, o1, o2, v1) extern DWORD vD2##v1;

#include "d2ptrs.inc"

#undef D2FUNCPTR
#undef D2FUNCPTR2
#undef D2VARPTR
#undef D2VARPTR2
#undef D2ASMPTR
#undef D2ASMPTR2

#endif
