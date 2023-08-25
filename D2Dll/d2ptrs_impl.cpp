#include "d2ptrs.h"

#define D2_PTR_IMPL

#define D2FUNCPTR(d1,o1,v1,t1,t2) D2##v1##_t *D2##v1 = (D2##v1##_t *)DLLOFFSET(d1,o1);
#define D2FUNCPTR2(d1,o1,o2,v1,t1,t2) D2##v1##_t *D2##v1 = (D2##v1##_t *)DLLOFFSET2(d1,o1,o2);
#define D2VARPTR(d1,o1,v1,t1)     p_D2##v1##_t *p_D2##v1 = (p_D2##v1##_t *)DLLOFFSET(d1,o1);
#define D2VARPTR2(d1,o1,o2,v1,t1)     p_D2##v1##_t *p_D2##v1 = (p_D2##v1##_t *)DLLOFFSET2(d1,o1,o2);
#define D2ASMPTR(d1,o1,v1)        volatile DWORD vD2##v1 = DLLOFFSET(d1,o1);
#define D2ASMPTR2(d1,o1,o2,v1)        DWORD vD2##v1 = DLLOFFSET2(d1,o1,o2);

#include "d2ptrs.inc"

DWORD CalcFuncOffset_X(DWORD dwNo, DWORD dwBase, int iValue)
{
    if ( 0 > iValue )
    {
        return (dwNo | (iValue <<8));
    }

    return (dwNo | ((iValue - dwBase) <<8));
}

DWORD Map_D2Version()
{
    return D2_VER_113C;
}
