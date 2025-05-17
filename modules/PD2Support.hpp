#pragma once

#include <stdint.h>
enum PD2ClickType {
    Left,
    Right,
};

#ifdef PD2_SUPPORT

static inline bool PD2Enabled()
{
    extern bool PD2IsEnabled;
    return PD2IsEnabled;
}
void PD2Setup(uintptr_t);

void PD2Click(PD2ClickType, bool shift, int x, int y);
int PD2GetFunc(unsigned char key);

#else
static inline bool PD2Enabled()
{
    return false;
}

static inline void PD2Setup(uintptr_t) {}
static inline void PD2Click(PD2ClickType, bool, int, int) {}
static inline int PD2GetFunc(unsigned char) { return -1; }

#endif
